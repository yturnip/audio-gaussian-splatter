//
// Created by Yohanes Turnip on 2026-08-04.
//

#ifndef AUDIOGAUSSIANSPLATTER_AUDIOENGINE_H
#define AUDIOGAUSSIANSPLATTER_AUDIOENGINE_H
#include <vector>
#include <memory>
#include <algorithm>
#include <juce_audio_basics/juce_audio_basics.h>
#include "ags/engine/SplatAudioProcessor.h"
#include "ags/manifold/GaussianManifold.h"
#include "ags/manifold/ManifoldRotator.h"

namespace ags::engine
{
    class AudioEngine
    {
    public:
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

        // Registers one splat's processor. The order processors are added
        // in must match the order splats appear in the manifold passed to
        // processSample(), since correspondence is purely positional. (needs some more information)
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

        [[nodiscard]] ags::engine::SplatAudioProcessor& getSplatProcessor(size_t index)
        {
            return *splatProcessors[index];
        }

        // Marks the cached rotated manifold dirty rather than rotating
        // immediately. Rotation angles change at parameter/GUI rate, not
        // per-sample, so the actual rotate() call in processSample() below
        // only re-runs when the angle actually changed (or a different
        // manifold instance is passed in) instead of on every single sample.
        void setRotation(const ags::manifold::RotationAngles& angles)
        {
            currentRotation = angles;
            rotationDirty = true;
        }

        // Runs every splat's processor over the same input sample, averaging
        // into mono output. The manifold is rotated at most once per distinct
        // (angles, manifold) pair via the cache below, not once per call.
        float processSample(float inputSample, const ags::manifold::GaussianManifold& manifold)
        {
            if (rotationDirty || cachedManifoldPtr != &manifold)
            {
                cachedRotatedManifold = rotator.rotate(manifold, currentRotation);
                cachedManifoldPtr = &manifold;
                rotationDirty = false;
            }

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

            scratchBuffer.resize(static_cast<size_t>(numSamples));

            for (size_t i = 0; i < channelsToFill; ++i)
            {
                std::copy(inputBuffer, inputBuffer + numSamples, scratchBuffer.begin());

                splatProcessors[i]->updateParametersForBlock(rotatedSplats[i]);
                splatProcessors[i]->processBlock(scratchBuffer.data(), numSamples, rotatedSplats[i]);

                std::copy(scratchBuffer.begin(), scratchBuffer.end(),
                           outputBuffer.getWritePointer(static_cast<int>(i)));
            }
        }


    private:
        void updateRotatedManifoldCacheIfNeeded(const ags::manifold::GaussianManifold& manifold)
        {
            if (rotationDirty || cachedManifoldPtr != &manifold)
            {
                cachedRotatedManifold = rotator.rotate(manifold, currentRotation);
                cachedManifoldPtr = &manifold;
                rotationDirty = false;
            }
        }

        std::vector<std::unique_ptr<SplatAudioProcessor>> splatProcessors;
        ags::manifold::ManifoldRotator rotator;
        ags::manifold::RotationAngles currentRotation;
        float currentSampleRate { 0.0f };

        ags::manifold::GaussianManifold cachedRotatedManifold;
        const ags::manifold::GaussianManifold* cachedManifoldPtr { nullptr };
        bool rotationDirty { true };

        std::vector<float> scratchBuffer;
    };
}
#endif //AUDIOGAUSSIANSPLATTER_AUDIOENGINE_H
