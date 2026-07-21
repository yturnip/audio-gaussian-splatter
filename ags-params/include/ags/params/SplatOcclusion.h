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
            const float nz = rotatedSplat.normal.z;
            return nz < minOcclusion ? minOcclusion : nz > maxOcclusion ? maxOcclusion : nz;
        }
    };
}
#endif //AUDIOGAUSSIANSPLATTER_SPLATOCCLUSION_H
