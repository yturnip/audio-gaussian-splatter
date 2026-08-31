//
// Created by Yohanes Turnip on 2026-08-31.
//

#ifndef AUDIOGAUSSIANSPLATTER_SPIRALVIBRATOPROCESSOR_H
#define AUDIOGAUSSIANSPLATTER_SPIRALVIBRATOPROCESSOR_H

#include "ags/engine/EffectProcessor.h"
#include "spiral/SpiralVibrato.h"

namespace ags::engine
{
    class SpiralVibratoProcessor : public EffectProcessor
    {
    public:
        // Parameter convention:
        // 0 = LFO frequency in Hz
        // 1 = depth in samples

        void setSampleRate(float newSampleRate) override
        {
            sampleRate = newSampleRate;
            vibrato.prepare(static_cast<double>(newSampleRate), maxDelaySamples);
        }

        void reset() override
        {
            vibrato.reset();
        }

        void setParameter(int paramId, float value) override
        {
            switch (paramId)
            {
                case 0: vibrato.setFrequency(value); break;
                case 1: vibrato.setDepthSamples(value); break;
                default: break;
            }
        }

        float processSample(float inputSample) override
        {
            return vibrato.processSample(inputSample);
        }

        [[nodiscard]] std::string getName() const override { return "Vibrato"; }

        [[nodiscard]] std::vector<EffectParameterDescriptor> getParameterDescriptors() const override
        {
            return {
                        { 0, "Rate", 0.1f, 15.0f, 5.0f, 0.0f },
                        { 1, "Depth", 1.0f, 100.0f, 10.0f, 0.0f }
            };
        }

    private:
        SpiralVibrato vibrato;
        float sampleRate { 44100.0f };
        static constexpr int maxDelaySamples = 4410; // 100ms at 44.1kHz
    };
}

#endif //AUDIOGAUSSIANSPLATTER_SPIRALVIBRATOPROCESSOR_H
