//
// Created by Yohanes Turnip on 2026-07-16.
//
#include "ags/params/EffectParameter.h"

#include <algorithm>

namespace ags::params
{
    EffectParameter::EffectParameter(float minValue, float maxValue, float defaultValue) :
    minValue(minValue), maxValue(maxValue), manualValue(std::clamp(defaultValue, minValue, maxValue))
    {
    }

    void EffectParameter::setManualValue(float value)
    {
        manualValue = std::clamp(value, minValue, maxValue);
    }

    void EffectParameter::setGMMDrivenValue(float normalizedValue)
    {
        const float clampedNormalized = std::clamp(normalizedValue, 0.0f, 1.0f);
        gmmDrivenValue = minValue + clampedNormalized * (maxValue - minValue);
        gmmBound = true;
    }

    void EffectParameter::clearGMMBinding()
    {
        gmmBound = false;
    }

    float EffectParameter::getValue() const
    {
        return gmmBound ? gmmDrivenValue : manualValue;
    }
}
