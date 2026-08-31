//
// Created by Yohanes Turnip on 2026-08-31.
//

#ifndef AUDIOGAUSSIANSPLATTER_SPIRALEXPANDERGATEPROCESSOR_H
#define AUDIOGAUSSIANSPLATTER_SPIRALEXPANDERGATEPROCESSOR_H

#include "ags/engine/EffectProcessor.h"
#include "spiral/SpiralExpanderGate.h"

namespace ags::engine
{
    class SpiralExpanderGateProcessor : public EffectProcessor
    {
    public:
        // Parameter convention:
        // 0 = threshold in dB
        // 1 = ratio (expansion ratio, >1.0 = more aggressive gating)
        // 2 = attack time in ms
        // 3 = release time in ms
        // 4 = floor gain in dB (minimum gain reduction when fully gated)
        // 5 = makeup gain in dB

        void setSampleRate(float newSampleRate) override
        {
            gate.setSampleRate(newSampleRate);
        }

        void reset() override
        {
            gate.reset();
        }

        void setParameter(int paramId, float value) override
        {
            switch (paramId)
            {
                case 0: gate.setThreshold(value); break;
                case 1: gate.setRatio(value); break;
                case 2: gate.setAttack(value); break;
                case 3: gate.setRelease(value); break;
                case 4: gate.setFloorGain(value); break;
                case 5: gate.setMakeUpGain(value); break;
                default: break;
            }
        }

        float processSample(float inputSample) override
        {
            const float gain = gate.computeGain(inputSample);
            return inputSample * gain;
        }

        [[nodiscard]] std::string getName() const override { return "Expander/Gate"; }

        [[nodiscard]] std::vector<EffectParameterDescriptor> getParameterDescriptors() const override
        {
            return {
                    { 0, "Threshold", -80.0f, 0.0f, -40.0f, 0.0f },
                    { 1, "Ratio", 1.0f, 10.0f, 2.0f, 0.0f },
                    { 2, "Attack", 0.1f, 200.0f, 10.0f, 0.0f },
                    { 3, "Release", 10.0f, 1000.0f, 100.0f, 0.0f },
                    { 4, "Floor Gain", -80.0f, 0.0f, -80.0f, 0.0f },
                    { 5, "Makeup Gain", 0.0f, 24.0f, 0.0f, 0.0f }
            };
        }

    private:
        SpiralExpanderGate gate;
    };
}

#endif //AUDIOGAUSSIANSPLATTER_SPIRALEXPANDERGATEPROCESSOR_H
