//
// Created by Yohanes Turnip on 2026-08-31.
//

#ifndef AUDIOGAUSSIANSPLATTER_SPIRALRINGMODPROCESSOR_H
#define AUDIOGAUSSIANSPLATTER_SPIRALRINGMODPROCESSOR_H

#include <juce_core/juce_core.h>
#include "ags/engine/EffectProcessor.h"
#include "spiral/SpiralRingMod.h"

namespace ags::engine
{
    class SpiralRingModProcessor : public EffectProcessor
    {
    public:
        // Parameter convention:
        // 0 = carrier frequency in Hz
        // 1 = depth [0.0 .. 1.0]
        // 2 = waveform (cast from float to SpiralRingMod::Waveform, 0-3)

        void setSampleRate(float newSampleRate) override
        {
            ringMod.setSampleRate(newSampleRate);
        }

        void reset() override
        {
            ringMod.reset();
        }

        void setParameter(int paramId, float value) override
        {
            switch (paramId)
            {
                case 0: ringMod.setCarrierFrequency(value); break;
                case 1: ringMod.setDepth(value); break;
                case 2:
                {
                    const int wf = juce::jlimit(0, 3, static_cast<int>(value));
                    ringMod.setWaveform(static_cast<SpiralRingMod::Waveform>(wf));
                    break;
                }
                default: break;
            }
        }

        float processSample(float inputSample) override
        {
            return ringMod.processSample(inputSample);
        }

        [[nodiscard]] std::string getName() const override { return "Ring Mod"; }

        [[nodiscard]] std::vector<EffectParameterDescriptor> getParameterDescriptors() const override
        {
            return {
                        { 0, "Carrier Freq", 1.0f, 5000.0f, 10.0f, 0.0f },
                        { 1, "Depth", 0.0f, 1.0f, 1.0f, 0.0f },
                        { 2, "Waveform", 0.0f, 3.0f, 0.0f, 1.0f }
            };
        }

    private:
        SpiralRingMod ringMod;
    };
}

#endif //AUDIOGAUSSIANSPLATTER_SPIRALRINGMODPROCESSOR_H
