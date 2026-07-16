//
// Created by Yohanes Turnip on 2026-07-16.
//

#ifndef AUDIOGAUSSIANSPLATTER_EFFECTPARAMETER_H
#define AUDIOGAUSSIANSPLATTER_EFFECTPARAMETER_H

namespace ags::params
{
    class EffectParameter
    {
    public:
        EffectParameter(float minValue, float maxValue, float defaultValue);

        void setManualValue(float value);
        void setGMMDrivenValue(float normalizedValue);
        void clearGMMBinding();

        [[nodiscard]] float getValue() const;
        [[nodiscard]] float isGMMBound() const {return gmmBound;}

        [[nodiscard]] float getMin() const {return minValue;}
        [[nodiscard]] float getMax() const {return maxValue;}

    private:
        float minValue;
        float maxValue;
        float manualValue;
        float gmmDrivenValue { 0.0f};
        bool gmmBound { false };
    };
}

#endif //AUDIOGAUSSIANSPLATTER_EFFECTPARAMETER_H
