//
// Created by Yohanes Turnip on 2026-07-02.
//

#ifndef AUDIOGAUSSIANSPLATTER_SPHEREBRANCHINGGENERATOR_H
#define AUDIOGAUSSIANSPLATTER_SPHEREBRANCHINGGENERATOR_H
#include "GeneratorConfig.h"
#include "ags/manifold/GaussianManifold.h"

namespace ags::manifold
{
    class SphereBranchingGenerator
    {
    public:
        [[nodiscard]] GaussianManifold generate(const GeneratorConfig& config) const;
    };
}

#endif //AUDIOGAUSSIANSPLATTER_SPHEREBRANCHINGGENERATOR_H