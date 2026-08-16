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

        EffectChain chain;
        ags::params::ParameterMapper mapper;
        std::vector<ParamSlot> slots;
    };
}

#endif //AUDIOGAUSSIANSPLATTER_SPLATAUDIOPROCESSOR_H
