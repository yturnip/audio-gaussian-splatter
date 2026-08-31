//
// Created by Yohanes Turnip on 2026-08-31.
//

#ifndef AUDIOGAUSSIANSPLATTER_SPIRALFLANGERPROCESSOR_H
#define AUDIOGAUSSIANSPLATTER_SPIRALFLANGERPROCESSOR_H

#include "ags/engine/EffectProcessor.h"
#include "spiral/SpiralFlanger.h"

namespace ags::engine
{
    class SpiralFlangerProcessor : public EffectProcessor
    {
    public:
        // Parameter convention:
        // 0 = LFO frequency in Hz
        // 1 = sweep width in samples
        // 2 = depth [0.0 .. 1.0]
        // 3 = feedback [0.0 .. 1.0]

        void setSampleRate(float newSampleRate) override
        {
            sampleRate = newSampleRate;
            flanger.prepare(static_cast<double>(newSampleRate), maxDelaySamples);
        }

        void reset() override
        {
            flanger.reset();
        }

        void setParameter(int paramId, float value) override
        {
            switch (paramId)
            {
                case 0: flanger.setFrequency(value); break;
                case 1: flanger.setSweepSamples(value); break;
                case 2: flanger.setDepth(value); break;
                case 3: flanger.setFeedback(value); break;
                default: break;
            }
        }

        float processSample(float inputSample) override
        {
            return flanger.processSample(inputSample);
        }

        [[nodiscard]] std::string getName() const override { return "Flanger"; }

        [[nodiscard]] std::vector<EffectParameterDescriptor> getParameterDescriptors() const override
        {
            return {
                        { 0, "Rate", 0.05f, 5.0f, 0.25f, 0.0f },
                        { 1, "Sweep", 1.0f, 200.0f, 10.0f, 0.0f },
                        { 2, "Depth", 0.0f, 1.0f, 0.7f, 0.0f },
                        { 3, "Feedback", 0.0f, 0.95f, 0.5f, 0.0f }
            };
        }

    private:
        SpiralFlanger flanger;
        float sampleRate { 44100.0f };
        static constexpr int maxDelaySamples = 4410; // 100ms at 44.1kHz, ample for sweep+depth
    };
}

#endif //AUDIOGAUSSIANSPLATTER_SPIRALFLANGERPROCESSOR_H
