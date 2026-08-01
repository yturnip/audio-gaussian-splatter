//
// Created by Yohanes Turnip on 2026-07-21.
//

#ifndef AUDIOGAUSSIANSPLATTER_SPLATOCCLUSION_H
#define AUDIOGAUSSIANSPLATTER_SPLATOCCLUSION_H
#include "ags/manifold/GaussianManifold.h"

namespace ags::params
{
    class SplatOcclusion
    {
    public:
        [[nodiscard]] static float compute(const ags::manifold::GaussianSplat& rotatedSplat,
                                            float minOcclusion = 0.3f,
                                            float maxOcclusion = 1.0f)
        {
            // Dynamic Occlusion
            // Dot product between rotated normal and listener (at 0,0,1)
            // High alignment = 1.0 (bright), misaligned = lower (dull)
            const float nz = rotatedSplat.normal.z;
            const float remmaped = (nz + 1.0f) * 0.5f;
            const float scaled = minOcclusion + remmaped * (maxOcclusion - minOcclusion);
            return std::clamp(scaled, minOcclusion, maxOcclusion);
        }
    };
}
#endif //AUDIOGAUSSIANSPLATTER_SPLATOCCLUSION_H
