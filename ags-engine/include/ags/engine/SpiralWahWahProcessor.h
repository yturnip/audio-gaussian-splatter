//
// Created by Yohanes Turnip on 2026-08-31.
//

#ifndef AUDIOGAUSSIANSPLATTER_SPIRALWAHWAHPROCESSOR_H
#define AUDIOGAUSSIANSPLATTER_SPIRALWAHWAHPROCESSOR_H

#include "ags/engine/EffectProcessor.h"
#include "spiral/SpiralWahWah.h"

namespace ags::engine
{
    class SpiralWahWahProcessor : public EffectProcessor
    {
    public:
        // Parameter convention:
        // 0 = mode select (0 = manual position, 1 = LFO auto-wah) -
        //     modeled as a float 0/1 like Tremolo's Waveform param
        // 1 = manual position [0..1] (used when paramId 0 < 0.5)
        // 2 = LFO rate in Hz (used when paramId 0 >= 0.5)
        // 3 = depth [0..1]

        void setSampleRate(float newSampleRate) override
        {
            sampleRate = newSampleRate;
            wahWah.setSampleRate(static_cast<double>(newSampleRate));

            wahWah.reset(1);
        }

        void reset() override
        {
            wahWah.reset(1);
        }

        void setParameter(int paramId, float value) override
        {
            switch (paramId)
            {
                case 0: isLfoMode = value >= 0.5f; break;
                case 1: manualPosition = value; if (!isLfoMode) wahWah.setManualPosition(value); break;
                case 2: if (isLfoMode) wahWah.setLfoRate(value); break;
                case 3: wahWah.setDepth(value); break;
                default: break;
            }
        }

        float processSample(float inputSample) override
        {
            return wahWah.processSample(inputSample, 0);
        }

        [[nodiscard]] std::string getName() const override { return "Wah-Wah"; }

        [[nodiscard]] std::vector<EffectParameterDescriptor> getParameterDescriptors() const override
        {
            return {
                    { 0, "Mode", 0.0f, 1.0f, 0.0f, 1.0f },
                    { 1, "Position", 0.0f, 1.0f, 0.5f, 0.0f },
                    { 2, "LFO Rate", 0.1f, 10.0f, 1.0f, 0.0f },
                    { 3, "Depth", 0.0f, 1.0f, 1.0f, 0.0f }
            };
        }

    private:
        SpiralWahWah wahWah;
        float sampleRate { 44100.0f };
        float manualPosition { 0.5f };
        bool isLfoMode { false };
    };
}

#endif //AUDIOGAUSSIANSPLATTER_SPIRALWAHWAHPROCESSOR_H
