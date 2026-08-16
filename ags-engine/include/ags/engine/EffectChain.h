//
// Created by Yohanes Turnip on 2026-07-25.
//

#ifndef AUDIOGAUSSIANSPLATTER_EFFECTCHAIN_H
#define AUDIOGAUSSIANSPLATTER_EFFECTCHAIN_H

#include <memory>
#include <vector>
#include "ags/engine/EffectProcessor.h"

namespace ags::engine
{
    class EffectChain
    {
    public:
        void setSampleRate(float sampleRate)
        {
            currentSampleRate = sampleRate;
            for (auto& slot : slots)
                slot.processor->setSampleRate(sampleRate);
        }

        void reset()
        {
            for (auto& slot : slots)
                slot.processor->reset();
        }

        size_t addEffect(std::unique_ptr<EffectProcessor> effect)
        {
            if (currentSampleRate > 0.0f)
                effect->setSampleRate(currentSampleRate);

            slots.push_back(Slot { std::move(effect) , false } );
            return slots.size() - 1;
        }

        void removeEffect(size_t index)
        {
            if (index < slots.size())
                slots.erase(slots.begin() + static_cast<long>(index));
        }

        void moveEffect(size_t fromIndex, size_t toIndex)
        {
            if (fromIndex >= slots.size() || toIndex >= slots.size())
                return;
            std::swap(slots[fromIndex], slots[toIndex]);
        }

        void setBypassed(size_t index, bool shouldBypass)
        {
            if (index < slots.size())
                slots[index].bypassed = shouldBypass;
        }

        [[nodiscard]] bool isBypassed(size_t index) const
        {
            return index < slots.size() && slots[index].bypassed;
        }

        void setParameter(size_t index, int paramId, float value)
        {
            if (index < slots.size())
                slots[index].processor->setParameter(paramId, value);
        }

        [[nodiscard]] size_t size() const
        {
            return slots.size();
        }

        float processSample(float inputSample)
        {
            float sample = inputSample;

            for (auto& slot : slots)
            {
                if (!slot.bypassed)
                    sample = slot.processor->processSample(sample);
            }

            return sample;
        }

        void processBlock(float* buffer, int numSamples)
        {
            for (auto& slot : slots)
            {
                if (!slot.bypassed)
                    slot.processor->processBlock(buffer, numSamples);
            }
        }
    private:
        struct Slot
        {
            std::unique_ptr<EffectProcessor> processor;
            bool bypassed;
        };

        std::vector<Slot> slots;
        float currentSampleRate { 0.0f };
    };
}
#endif //AUDIOGAUSSIANSPLATTER_EFFECTCHAIN_H
