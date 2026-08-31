//
// Created by Yohanes Turnip on 2026-07-25.
//

#ifndef AUDIOGAUSSIANSPLATTER_AUDIOENGINE_H
#define AUDIOGAUSSIANSPLATTER_AUDIOENGINE_H
#include <vector>
#include <memory>
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include "ags/engine/SplatAudioProcessor.h"
#include "ags/manifold/GaussianManifold.h"
#include "ags/manifold/ManifoldRotator.h"

namespace ags::engine
{
    class AudioEngine
    {
    public:
        AudioEngine()
        {
            const int hardwareThreads = juce::SystemStats::getNumCpus();
            numWorkerThreads = juce::jlimit(0, 15, hardwareThreads - 1);

            for (int i = 0; i < numWorkerThreads; ++i)
                workers.push_back(std::make_unique<Worker>(*this, i));
        }

        ~AudioEngine()
        {
            shuttingDown.store(true);
            for (auto& worker : workers)
                worker->wakeAndJoin();
        }

        void setSampleRate(float sampleRate)
        {
            currentSampleRate = sampleRate;
            for (auto& processor : splatProcessors)
                processor->setSampleRate(sampleRate);
        }

        void reset()
        {
            for (auto& processor : splatProcessors)
                processor->reset();
        }

        size_t addSplatProcessor(std::unique_ptr<SplatAudioProcessor> processor)
        {
            if (currentSampleRate > 0.0f)
                processor->setSampleRate(currentSampleRate);

            splatProcessors.push_back(std::move(processor));
            return splatProcessors.size() - 1;
        }

        [[nodiscard]] size_t size() const
        {
            return splatProcessors.size();
        }

        [[nodiscard]] SplatAudioProcessor& getSplatProcessor(size_t index)
        {
            return *splatProcessors[index];
        }

        void setParameterValueForAll(size_t effectIndex, int paramIndex, float value)
        {
            for (auto& processor : splatProcessors)
                processor->setParameterValue(effectIndex, paramIndex, value);
        }

        void setParameterBindingForAll(size_t effectIndex, int paramIndex, ags::params::GMMBinding binding)
        {
            for (auto& processor : splatProcessors)
                processor->setParameterBinding(effectIndex, paramIndex, binding);
        }

        template <typename EffectFactory>
        size_t addEffectForAll(EffectFactory&& effectFactory)
        {
            size_t addedIndex = 0;
            bool first = true;
            for (auto& processor : splatProcessors)
            {
                auto effect = effectFactory();
                const auto descriptors = effect->getParameterDescriptors();
                const auto index = processor->addEffect(std::move(effect));

                for (const auto& descriptor : descriptors)
                {
                    processor->addParameterSlot(
                        index, descriptor.paramId,
                        std::make_unique<ags::params::EffectParameter>(
                            descriptor.minValue, descriptor.maxValue, descriptor.defaultValue),
                            ags::params::GMMBinding {ags::params::GMMAttribute::None, false });
                }
                if (first)
                {
                    addedIndex = index;
                    first = false;
                }
                jassert(index == addedIndex);
            }
            return addedIndex;
        }

        void removeEffectForAll(size_t effectIndex)
        {
            for (auto& processor : splatProcessors)
                processor->removeEffect(effectIndex);
        }

        void moveEffectForAll(size_t fromIndex, size_t newIndex)
        {
            for (auto& processor : splatProcessors)
                processor->moveEffect(fromIndex, newIndex);
        }

        void setBypassedForAll(size_t effectIndex, bool shouldBypassed)
        {
            for (auto& processor : splatProcessors)
                processor->setBypassed(effectIndex, shouldBypassed);
        }

        void setRotation(const ags::manifold::RotationAngles& angles)
        {
            currentRotation = angles;
        }

        float processSample(float inputSample, const ags::manifold::GaussianManifold& manifold)
        {
            updateRotatedManifoldCacheIfNeeded(manifold);
            const auto& rotatedSplats = cachedRotatedManifold.splats();

            jassert(splatProcessors.size() == rotatedSplats.size());
            const size_t count = std::min(splatProcessors.size(), rotatedSplats.size());

            float sum = 0.0f;
            for (size_t i = 0; i < count; ++i)
            {
                splatProcessors[i]->updateParametersForBlock(rotatedSplats[i]);
                sum += splatProcessors[i]->processSample(inputSample, rotatedSplats[i]);
            }

            return count > 0 ? sum / static_cast<float>(count) : inputSample;
        }

        void processBlock(const float* inputBuffer, int numSamples,
                           const ags::manifold::GaussianManifold& manifold,
                           juce::AudioBuffer<float>& outputBuffer)
        {
            updateRotatedManifoldCacheIfNeeded(manifold);
            const auto& rotatedSplats = cachedRotatedManifold.splats();

            const size_t count = std::min(splatProcessors.size(), rotatedSplats.size());
            const size_t availableChannels = static_cast<size_t>(outputBuffer.getNumChannels());
            const size_t channelsToFill = std::min(count, availableChannels);

            if (channelsToFill == 0)
                return;

            const int totalRanges = numWorkerThreads + 1;
            const size_t rangeSize = (channelsToFill + static_cast<size_t>(totalRanges) - 1)
                                      / static_cast<size_t>(totalRanges);

            currentBlockContext = BlockContext {
                inputBuffer, numSamples, &rotatedSplats, &outputBuffer,
                channelsToFill, rangeSize
            };

            pendingWorkers.store(numWorkerThreads);

            const uint64_t thisGeneration = ++blockGeneration;
            for (auto& worker : workers)
                worker->wakeForBlock(thisGeneration);

            const size_t lastRangeIndex = static_cast<size_t>(numWorkerThreads);
            processRange(lastRangeIndex);

            while (pendingWorkers.load(std::memory_order_acquire) > 0)
                std::this_thread::yield();
        }

    private:
        struct BlockContext
        {
            const float* inputBuffer { nullptr };
            int numSamples { 0 };
            const std::vector<ags::manifold::GaussianSplat>* rotatedSplats { nullptr };
            juce::AudioBuffer<float>* outputBuffer { nullptr };
            size_t channelsToFill { 0 };
            size_t rangeSize { 0 };
        };

        void processRange(size_t rangeIndex)
        {
            const auto& ctx = currentBlockContext;
            if (ctx.outputBuffer == nullptr)
                return;

            const size_t begin = rangeIndex * ctx.rangeSize;
            const size_t end = std::min(begin + ctx.rangeSize, ctx.channelsToFill);

            for (size_t i = begin; i < end; ++i)
            {
                auto* channelData = ctx.outputBuffer->getWritePointer(static_cast<int>(i));
                std::copy(ctx.inputBuffer, ctx.inputBuffer + ctx.numSamples, channelData);

                splatProcessors[i]->updateParametersForBlock((*ctx.rotatedSplats)[i]);
                splatProcessors[i]->processBlock(channelData, ctx.numSamples, (*ctx.rotatedSplats)[i]);
            }
        }

        void updateRotatedManifoldCacheIfNeeded(const ags::manifold::GaussianManifold& manifold)
        {
            if (rotationDirty || cachedManifoldPtr != &manifold)
            {
                cachedRotatedManifold = rotator.rotate(manifold, currentRotation);
                cachedManifoldPtr = &manifold;
                rotationDirty = false;
            }
        }

        class Worker final : public juce::Thread
        {
        public:
            Worker(AudioEngine& ownerEngine, int rangeIdx)
                : juce::Thread("AGS AudioEngine Worker " + juce::String(rangeIdx)),
                  owner(ownerEngine), rangeIndex(static_cast<size_t>(rangeIdx))
            {
                startThread(juce::Thread::Priority::high);
            }

            ~Worker() override
            {
                stopThread(2000);
            }

            void wakeForBlock(uint64_t generation)
            {
                {
                    std::lock_guard<std::mutex> lock(wakeMutex);
                    targetGeneration = generation;
                }
                wakeCondition.notify_one();
            }

            void wakeAndJoin()
            {
                signalThreadShouldExit();
                wakeCondition.notify_one();
            }

            void run() override
            {
                uint64_t lastProcessedGeneration = 0;

                while (!threadShouldExit())
                {
                    std::unique_lock<std::mutex> lock(wakeMutex);
                    wakeCondition.wait(lock, [this, &lastProcessedGeneration]
                    {
                        return threadShouldExit() || targetGeneration > lastProcessedGeneration;
                    });

                    if (threadShouldExit())
                        return;

                    lastProcessedGeneration = targetGeneration;
                    lock.unlock();

                    owner.processRange(rangeIndex);
                    owner.pendingWorkers.fetch_sub(1, std::memory_order_release);
                }
            }

        private:
            AudioEngine& owner;
            size_t rangeIndex;

            std::mutex wakeMutex;
            std::condition_variable wakeCondition;
            uint64_t targetGeneration { 0 };
        };

        std::vector<std::unique_ptr<SplatAudioProcessor>> splatProcessors;
        ags::manifold::ManifoldRotator rotator;
        ags::manifold::RotationAngles currentRotation;
        float currentSampleRate { 0.0f };

        ags::manifold::GaussianManifold cachedRotatedManifold;
        const ags::manifold::GaussianManifold* cachedManifoldPtr { nullptr };
        bool rotationDirty { true };

        int numWorkerThreads { 0 };
        std::vector<std::unique_ptr<Worker>> workers;
        std::atomic<int> pendingWorkers { 0 };
        std::atomic<uint64_t> blockGeneration { 0 };
        std::atomic<bool> shuttingDown { false };

        BlockContext currentBlockContext;

        JUCE_DECLARE_NON_COPYABLE(AudioEngine)
        JUCE_DECLARE_NON_MOVEABLE(AudioEngine)
    };
}
#endif //AUDIOGAUSSIANSPLATTER_AUDIOENGINE_H
