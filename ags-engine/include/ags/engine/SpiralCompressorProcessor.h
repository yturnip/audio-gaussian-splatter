//
// Created by Yohanes Turnip on 2026-08-31.
//

#ifndef AUDIOGAUSSIANSPLATTER_SPIRALCOMPRESSORPROCESSOR_H
#define AUDIOGAUSSIANSPLATTER_SPIRALCOMPRESSORPROCESSOR_H

#include "ags/engine/EffectProcessor.h"
#include "spiral/SpiralCompressor.h"

namespace ags::engine
{
    class SpiralCompressorProcessor : public EffectProcessor
    {
    public:
        // Parameter convention:
        // 0 = threshold in dB
        // 1 = ratio (1.0 = no compression)
        // 2 = attack time in ms
        // 3 = release time in ms
        // 4 = makeup gain in dB

        void setSampleRate(float newSampleRate) override
        {
            compressor.setSampleRate(newSampleRate);
        }

        void reset() override
        {
            compressor.reset();
        }

        void setParameter(int paramId, float value) override
        {
            switch (paramId)
            {
                case 0: compressor.setThreshold(value); break;
                case 1: compressor.setRatio(value); break;
                case 2: compressor.setAttack(value); break;
                case 3: compressor.setRelease(value); break;
                case 4: compressor.setMakeUpGain(value); break;
                default: break;
            }
        }

        float processSample(float inputSample) override
        {
            const float gain = compressor.computeGain(inputSample);
            return inputSample * gain;
        }

        [[nodiscard]] std::string getName() const override { return "Compressor"; }

        [[nodiscard]] std::vector<EffectParameterDescriptor> getParameterDescriptors() const override
        {
            return {
                    { 0, "Threshold", -60.0f, 0.0f, -18.0f, 0.0f },
                    { 1, "Ratio", 1.0f, 20.0f, 4.0f, 0.0f },
                    { 2, "Attack", 0.1f, 200.0f, 10.0f, 0.0f },
                    { 3, "Release", 10.0f, 1000.0f, 100.0f, 0.0f },
                    { 4, "Makeup Gain", 0.0f, 24.0f, 0.0f, 0.0f }
            };
        }

    private:
        SpiralCompressor compressor;
    };
}

#endif //AUDIOGAUSSIANSPLATTER_SPIRALCOMPRESSORPROCESSOR_H
