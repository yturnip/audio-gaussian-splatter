//
// Created by Yohanes Turnip on 2026-07-25.
//

#ifndef AUDIOGAUSSIANSPLATTER_SPLATAUDIOPROCESSOR_H
#define AUDIOGAUSSIANSPLATTER_SPLATAUDIOPROCESSOR_H

#include <vector>
#include <memory>
#include <algorithm>
#include "ags/engine/EffectChain.h"
#include "ags/params/EffectParameter.h"
#include "ags/params/GMMBinding.h"
#include "ags/params/ParameterMapper.h"
#include "ags/params/SplatOcclusion.h"
#include "ags/manifold/GaussianSplat.h"

namespace ags::engine
{
    class SplatAudioProcessor
    {
    public:
        void setSampleRate(float sampleRate)
        {
            const ScopedStructuralEdit edit(*this);
            chain.setSampleRate(sampleRate);
        }

        void reset()
        {
            const ScopedStructuralEdit edit(*this);
            chain.reset();
        }

        void addParameterSlot(size_t effectIndex, int paramIndex,
                                std::unique_ptr<ags::params::EffectParameter> parameter,
                                ags::params::GMMBinding binding)
        {
            const ScopedStructuralEdit edit(*this);
            slots.push_back(ParamSlot {
                effectIndex,
                paramIndex,
                std::move(parameter),
                binding
            });
        }

        size_t addEffect(std::unique_ptr<EffectProcessor> effect)
        {
            const ScopedStructuralEdit edit(*this);
            return chain.addEffect(std::move(effect));
        }

        void removeEffect(size_t effectIndex)
        {
            const ScopedStructuralEdit edit(*this);
            chain.removeEffect(effectIndex);

            slots.erase(std::remove_if(slots.begin(), slots.end(), [effectIndex](const ParamSlot& s)
                { return s.effectIndex == effectIndex; }), slots.end());

            for (auto& slot : slots)
            {
                if (slot.effectIndex > effectIndex)
                    --slot.effectIndex;
            }
        }

        void moveEffect(size_t fromIndex, size_t toIndex)
        {
            if (fromIndex == toIndex)
                return;
            const ScopedStructuralEdit edit(*this);

            chain.moveEffect(fromIndex, toIndex);

            for (auto& slot : slots)
            {
                if (slot.effectIndex == fromIndex)
                    slot.effectIndex = toIndex;
                else if (slot.effectIndex == toIndex)
                    slot.effectIndex = fromIndex;
            }
        }

        void setBypassed(size_t effectIndex, bool shouldBypass)
        {
            const ScopedStructuralEdit edit(*this);
            chain.setBypassed(effectIndex, shouldBypass);
        }

        [[nodiscard]] bool isEffectBypassed(size_t effectIndex) const
        {
            const juce::ScopedLock lock(structureLock);
            return chain.isBypassed(effectIndex);
        }

        [[nodiscard]] size_t getEffectCount() const
        {
            const juce::ScopedLock lock(structureLock);
            return chain.size();
        }

        [[nodiscard]] std::string getEffectName(size_t effectIndex) const
        {
            const juce::ScopedLock lock(structureLock);
            return chain.getEffect(effectIndex).getName();
        }

        [[nodiscard]] std::vector<EffectParameterDescriptor> getEffectDescriptors(size_t effectIndex) const
        {
            const juce::ScopedLock lock(structureLock);
            return chain.getEffect(effectIndex).getParameterDescriptors();
        }

        struct ParamSlotView
        {
            size_t effectIndex;
            int paramIndex;
            float minValue;
            float maxValue;
            float currentValue;
            ags::params::GMMBinding binding;
        };

        [[nodiscard]] ParamSlotView getParameterSlotView(size_t effectIndex, int paramIndex) const
        {
            const juce::ScopedLock lock(structureLock);
            const auto it = findSlot(effectIndex, paramIndex);
            jassert(it != slots.end());
            return ParamSlotView{
                it->effectIndex, it->paramIndex,
                it->parameter->getMin(), it->parameter->getMax(),
                it->parameter->getValue(), it->binding
            };
        }

        void setParameterBinding(size_t effectIndex, int paramIndex, ags::params::GMMBinding newBinding)
        {
            const ScopedStructuralEdit edit(*this);
            const auto it = findSlot(effectIndex, paramIndex);
            jassert(it != slots.end());
            it->binding = newBinding;
        }

        void setParameterValue(size_t effectIndex, int paramIndex, float value)
        {
            const ScopedStructuralEdit edit(*this);
            const auto it = findSlot(effectIndex, paramIndex);
            jassert(it != slots.end());
            it->parameter->setManualValue(value);
            chain.setParameter(it->effectIndex, it->paramIndex, it->parameter->getValue());
        }

        void updateParametersForBlock(const ags::manifold::GaussianSplat& splat)
        {
            const juce::SpinLock::ScopedTryLockType lock(audioLock);
            if (!lock.isLocked())
                return;

            for (auto& slot : slots)
            {
                mapper.apply(*slot.parameter, slot.binding, splat);
                chain.setParameter(slot.effectIndex, slot.paramIndex, slot.parameter->getValue());
            }
        }

        float processSample(float inputSample, const ags::manifold::GaussianSplat& rotatedSplat)
        {
            const juce::SpinLock::ScopedTryLockType lock(audioLock);
            if (!lock.isLocked())
                return inputSample;

            const float wet = chain.processSample(inputSample);
            const float gain = ags::params::SplatOcclusion::compute(rotatedSplat);
            return wet * gain;
        }

        void processBlock(float* buffer, int numSamples, const ags::manifold::GaussianSplat& rotatedSplat)
        {
            const juce::SpinLock::ScopedTryLockType lock(audioLock);
            if (!lock.isLocked())
                return; // buffer left as-is: passthrough for this block only

            chain.processBlock(buffer, numSamples);

            const float gain = ags::params::SplatOcclusion::compute(rotatedSplat);
            for (int i = 0; i < numSamples; ++i)
                buffer[i] *= gain;
        }

    private:
        struct ParamSlot
        {
            size_t effectIndex;
            int paramIndex;
            std::unique_ptr<ags::params::EffectParameter> parameter;
            ags::params::GMMBinding binding;
        };

        [[nodiscard]] std::vector<ParamSlot>::iterator findSlot(size_t effectIndex, int paramIndex)
        {
            return std::find_if(slots.begin(), slots.end(), [&](const ParamSlot& s) {
                return s.effectIndex == effectIndex && s.paramIndex == paramIndex;
            });
        }

        [[nodiscard]] std::vector<ParamSlot>::const_iterator findSlot(size_t effectIndex, int paramIndex) const
        {
            return std::find_if(slots.begin(), slots.end(), [&](const ParamSlot& s) {
                return s.effectIndex == effectIndex && s.paramIndex == paramIndex;
            });
        }

        struct ScopedStructuralEdit
        {
            explicit ScopedStructuralEdit(SplatAudioProcessor& owner)
                : audioScopedLock(owner.audioLock),
                  structureScopedLock(owner.structureLock)
            {}

            juce::SpinLock::ScopedLockType audioScopedLock;
            juce::ScopedLock structureScopedLock;
        };

        juce::SpinLock audioLock;

        mutable juce::CriticalSection structureLock;

        EffectChain chain;
        ags::params::ParameterMapper mapper;
        std::vector<ParamSlot> slots;
    };
}
#endif //AUDIOGAUSSIANSPLATTER_SPLATAUDIOPROCESSOR_H
