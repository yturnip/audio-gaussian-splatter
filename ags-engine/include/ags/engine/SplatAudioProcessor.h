//
// Created by Yohanes Turnip on 2026-07-30.
//

#ifndef AUDIOGAUSSIANSPLATTER_SPLATAUDIOPROCESSOR_H
#define AUDIOGAUSSIANSPLATTER_SPLATAUDIOPROCESSOR_H

#include <vector>
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
        void setSampleRate(float sampleRate) { chain.setSampleRate(sampleRate); }

        void reset() { chain.reset(); }

        // Registers one parameter slot with its GMM Binding
        // paramIndex refers to the effect's own parameter ID inside the chain
        void addParameterSlot(size_t effectIndex,
                                int paramIndex,
                                std::unique_ptr<ags::params::EffectParameter> parameter,
                                ags::params::GMMBinding binding)
        {
            slots.push_back(ParamSlot {
                effectIndex,
                paramIndex,
                std::move(parameter),
                binding
            });
        }

        size_t addEffect(std::unique_ptr<EffectProcessor> effect)
        {
            return chain.addEffect(std::move(effect));
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

        // Looked up by (effectIndex, paramIndex) identity, not flat position --
        // stays correct regardless of how many effects are chained.
        [[nodiscard]] ParamSlotView getParameterSlotView(size_t effectIndex, int paramIndex) const
        {
            const auto it = findSlot(effectIndex, paramIndex);
            jassert(it != slots.end());
            return ParamSlotView{
                it->effectIndex, it->paramIndex,
                it->parameter->getMin(), it->parameter->getMax(),
                it->parameter->getValue(), it->binding
            };
        }

        // Changes which GMM attribute (or None) drives this parameter. Does not
        // touch EffectParameter directly -- ParameterMapper::apply already
        // reconciles EffectParameter's internal gmmBound flag against
        // ParamSlot::binding every block (calling clearGMMBinding() when
        // !binding.isActive(), setGMMDrivenValue() otherwise), so the very next
        // updateParametersForBlock call after this picks up the new binding.
        void setParameterBinding(size_t effectIndex, int paramIndex, ags::params::GMMBinding newBinding)
        {
            const auto it = findSlot(effectIndex, paramIndex);
            jassert(it != slots.end());
            it->binding = newBinding;
        }

        // Directly sets a parameter's manual value. Only meaningful while the
        // slot is unbound -- if a GMM attribute is bound, the next
        // updateParametersForBlock call overwrites this via setGMMDrivenValue
        // regardless.
        void setParameterValue(size_t effectIndex, int paramIndex, float value)
        {
            const auto it = findSlot(effectIndex, paramIndex);
            jassert(it != slots.end());
            it->parameter->setManualValue(value);
            chain.setParameter(it->effectIndex, it->paramIndex, it->parameter->getValue());
        }

        // Reads the splat's current GMM attributes, maps them through
        // ParameterMapper, and pushes the resulting values into the chain.
        void updateParametersForBlock(const ags::manifold::GaussianSplat& splat)
        {
            for (auto& slot : slots)
            {
                mapper.apply(*slot.parameter, slot.binding, splat);
                chain.setParameter(slot.effectIndex, slot.paramIndex, slot.parameter->getValue());
            }
        }

        // Processes one sample through the chain and applies occlusion gain
        float processSample(float inputSample, const::ags::manifold::GaussianSplat& rotatedSplat)
        {
            const float wet = chain.processSample(inputSample);
            const float gain = ags::params::SplatOcclusion::compute(rotatedSplat);
            return wet * gain;
        }

        void processBlock(float* buffer, int numSamples, const ags::manifold::GaussianSplat& rotatedSplat)
        {
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

        EffectChain chain;
        ags::params::ParameterMapper mapper;
        std::vector<ParamSlot> slots;
    };
}

#endif //AUDIOGAUSSIANSPLATTER_SPLATAUDIOPROCESSOR_H
