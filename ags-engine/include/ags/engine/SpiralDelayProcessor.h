//
// Created by Yohanes Turnip on 2026-07-25.
//

#ifndef AUDIOGAUSSIANSPLATTER_SPIRALDELAYPROCESSOR_H
#define AUDIOGAUSSIANSPLATTER_SPIRALDELAYPROCESSOR_H

#include "EffectProcessor.h"
#include "spiral/SpiralDelay.h"

namespace ags::engine
{
    class SpiralDelayProcessor : public EffectProcessor
    {
    public:
        // Parameter convention:
        // 0 = delay time in samples (int, cast from float)
        // 1 = feedback [0.0 .. 1.0]
        // 2 = wet mix  [0.0 .. 1.0]

        void setSampleRate(float newSampleRate) override
        {
            sampleRate = newSampleRate;
            delay.prepare(static_cast<double>(sampleRate), maxDelaySamples);
        }

        void reset() override
        {
            delay.reset();
        }

        void setParameter(int paramId, float value) override
        {
            switch (paramId)
            {
                case 0: delay.setDelaySamples(static_cast<int>(value)); break;
                case 1: delay.setFeedback(value); break;
                case 2: delay.setWet(value); break;
                default: break;
            }
        }

        float processSample(float inputSample) override
        {
            return delay.processSample(inputSample);
        }
    private:
        SpiralDelay delay;
        float sampleRate { 44100.0f };
        static constexpr int maxDelaySamples = 88200;
    };
}
#endif //AUDIOGAUSSIANSPLATTER_SPIRALDELAYPROCESSOR_H
