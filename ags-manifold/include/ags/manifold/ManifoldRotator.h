//
// Created by Yohanes Turnip on 2026-07-09.
//

#ifndef AUDIOGAUSSIANSPLATTER_MANIFOLDROTATOR_H
#define AUDIOGAUSSIANSPLATTER_MANIFOLDROTATOR_H
#include "ags/manifold/GaussianManifold.h"

namespace ags::manifold
{
    struct RotationAngles
    {
        float yawRadians    {0.0f}; // rotation around Y axis
        float pitchRadians  {0.0f}; // rotation around X axis
        float rollRadians   {0.0f}; // rotation around Z axis
    };

    class ManifoldRotator
    {
    public:
        [[nodiscard]] GaussianManifold rotate (const GaussianManifold& manifold, const RotationAngles& rotationAngles) const;
    };
}
#endif //AUDIOGAUSSIANSPLATTER_MANIFOLDROTATOR_H
