//
// Created by Yohanes Turnip on 2026-07-25.
//

#ifndef AUDIOGAUSSIANSPLATTER_EFFECTPROCESSOR_H
#define AUDIOGAUSSIANSPLATTER_EFFECTPROCESSOR_H

namespace ags::engine
{
    struct EffectParameterDescriptor
    {
        int paramId { 0 };
        std::string name;
        float minValue { 0.0f };
        float maxValue { 1.0f };
        float defaultValue { 0.0f };

        float stepSize { 0.0f };
    };

    class EffectProcessor
    {
    public:
        virtual ~EffectProcessor() = default;

        virtual void setSampleRate(float sampleRate) = 0;
        virtual void reset() = 0;
        virtual void setParameter(int paramId, float value) = 0;

        virtual float processSample(float inputSample) = 0;

        virtual void processBlock(float* buffer, int numSamples)
        {
            for (int i = 0; i < numSamples; ++i)
                buffer[i] = processSample(buffer[i]);
        }

        [[nodiscard]] virtual std::string getName() const = 0;
        [[nodiscard]] virtual std::vector<EffectParameterDescriptor> getParameterDescriptors() const = 0;
    };
}
#endif //AUDIOGAUSSIANSPLATTER_EFFECTPROCESSOR_H
