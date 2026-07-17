//
// Created by Yohanes Turnip on 2026-07-17.
//
#include "ags/params/ParameterMapper.h"
#include <algorithm>

namespace ags::params
{
    float ParameterMapper::readNormalizedAttribute(GMMAttribute attribute,
        const ags::manifold::GaussianSplat& splat)
    {
        switch (attribute)
        {
            case GMMAttribute::Density:
                return std::clamp(splat.density, 0.0f, 1.0f);

            case GMMAttribute::ShColor:
                return std::clamp(splat.shColor, 0.0f, 1.0f);

            case GMMAttribute::Eccentricity:
                return std::clamp((splat.eccentricity - 1.0f) / 9.0f, 0.0f, 1.0f);

            case GMMAttribute::Curvature:
                return std::clamp((splat.curvature + 1.0f) * 0.5f, 0.0f, 1.0f);

            case GMMAttribute::None:
            default:
                return 0.0f;
        }
    }

    void ParameterMapper::apply(EffectParameter& parameter,
                                 const GMMBinding& binding,
                                 const ags::manifold::GaussianSplat& splat) const
    {
        if (!binding.isActive())
        {
            parameter.clearGMMBinding();
            return;
        }

        float normalized = readNormalizedAttribute(binding.attribute, splat);

        if (binding.invert)
            normalized = 1.0f - normalized;

        parameter.setGMMDrivenValue(normalized);
    }
}
