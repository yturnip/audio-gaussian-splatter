//
// Created by Yohanes Turnip on 2026-07-25.
//

#ifndef AUDIOGAUSSIANSPLATTER_SPIRALTREMOLOPROCESSOR_H
#define AUDIOGAUSSIANSPLATTER_SPIRALTREMOLOPROCESSOR_H

#include <juce_core/juce_core.h>
#include "ags/engine/EffectProcessor.h"
#include "spiral/SpiralTremolo.h"

namespace ags::engine
{
    class SpiralTremoloProcessor : public EffectProcessor
    {
    public:
        // Parameter convention:
        // 0 = frequency in Hz
        // 1 = depth [0.0 .. 1.0]
        // 2 = waveform (cast from float to SpiralTremolo::Waveform, 0-3)

        void setSampleRate(float newSampleRate) override
        {
            tremolo.setSampleRate(newSampleRate);
        }

        void reset() override
        {
            tremolo.reset();
        }

        void setParameter(int paramId, float value) override
        {
            switch (paramId)
            {
                case 0: tremolo.setFrequency(value); break;
                case 1: tremolo.setDepth(value); break;
                case 2:
                {
                    const int wf = juce::jlimit(0, 3, static_cast<int>(value));
                    tremolo.setWaveform(static_cast<SpiralTremolo::Waveform>(wf));
                    break;
                }
                default: break;
            }
        }

        float processSample(float inputSample) override
        {
            return tremolo.processSample(inputSample);
        }

        [[nodiscard]] std::string getName() const override { return "Tremolo"; }

        [[nodiscard]] std::vector<EffectParameterDescriptor> getParameterDescriptors() const override
        {
            return {
                    { 0, "Rate", 0.1f, 20.0f, 5.0f },
                    { 1, "Depth", 0.0f, 1.0f, 0.0f },
                    { 2, "Waveform", 0.0f, 3.0f, 0.0f }
            };
        }

    private:
        SpiralTremolo tremolo;
    };
}

#endif //AUDIOGAUSSIANSPLATTER_SPIRALTREMOLOPROCESSOR_H