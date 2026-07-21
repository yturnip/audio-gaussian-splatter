//
// Created by Yohanes Turnip on 2026-07-17.
//

#ifndef AUDIOGAUSSIANSPLATTER_PARAMETERMAPPER_H
#define AUDIOGAUSSIANSPLATTER_PARAMETERMAPPER_H
#include <vector>
#include "ags/params/EffectParameter.h"
#include "ags/params/GMMBinding.h"
#include "ags/manifold/GaussianSplat.h"

namespace ags::params
{
    class ParameterMapper
    {
    public:
        void apply(EffectParameter& parameter,
                    const GMMBinding& binding,
                    const ags::manifold::GaussianSplat& splat) const;

        void applyToMany(const std::vector<EffectParameter*>& parameters,
                            const GMMBinding& binding,
                            const ags::manifold::GaussianSplat& splat) const;

    private:
        [[nodiscard]] static float readNormalizedAttribute(GMMAttribute attribute,
            const ags::manifold::GaussianSplat& splat);
    };
}
#endif //AUDIOGAUSSIANSPLATTER_PARAMETERMAPPER_H
