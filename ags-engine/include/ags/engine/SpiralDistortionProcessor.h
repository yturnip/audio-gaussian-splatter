//
// Created by Yohanes Turnip on 2026-08-31.
//

#ifndef AUDIOGAUSSIANSPLATTER_SPIRALDISTORTIONPROCESSOR_H
#define AUDIOGAUSSIANSPLATTER_SPIRALDISTORTIONPROCESSOR_H

#include <juce_core/juce_core.h>
#include "ags/engine/EffectProcessor.h"
#include "spiral/SpiralDistortion.h"

namespace ags::engine
{
    class SpiralDistortionProcessor : public EffectProcessor
    {
    public:
        // Parameter convention:
        // 0 = distortion type (cast from float to SpiralDistortion::DistortionType, 0-4)
        // 1 = input gain in dB
        // 2 = output gain in dB

        void setSampleRate(float newSampleRate) override
        {
            distortion.setSampleRate(newSampleRate);
        }

        void reset() override
        {
            distortion.reset();
        }

        void setParameter(int paramId, float value) override
        {
            switch (paramId)
            {
                case 0:
                {
                    const int type = juce::jlimit(0, 4, static_cast<int>(value));
                    distortion.setDistortionType(static_cast<SpiralDistortion::DistortionType>(type));
                    break;
                }
                case 1: distortion.setInputGainDb(value); break;
                case 2: distortion.setOutputGainDb(value); break;
                default: break;
            }
        }

        float processSample(float inputSample) override
        {
            return distortion.processSample(inputSample);
        }

        [[nodiscard]] std::string getName() const override { return "Distortion"; }

        [[nodiscard]] std::vector<EffectParameterDescriptor> getParameterDescriptors() const override
        {
            return {
                        { 0, "Type", 0.0f, 4.0f, 0.0f, 1.0f },
                        { 1, "Input Gain", -24.0f, 24.0f, 0.0f, 0.0f },
                        { 2, "Output Gain", -24.0f, 24.0f, 0.0f, 0.0f }
            };
        }

    private:
        SpiralDistortion distortion;
    };
}

#endif //AUDIOGAUSSIANSPLATTER_SPIRALDISTORTIONPROCESSOR_H
