//
// Created by Yohanes Turnip on 2026-07-02.
//

#ifndef AUDIOGAUSSIANSPLATTER_DOMEBRANCHINGGENERATOR_H
#define AUDIOGAUSSIANSPLATTER_DOMEBRANCHINGGENERATOR_H
#include "GeneratorConfig.h"
#include "ags/manifold/GaussianManifold.h"

#endif //AUDIOGAUSSIANSPLATTER_DOMEBRANCHINGGENERATOR_H

namespace ags::manifold
{
    class DomeBranchingGenerator
    {
    public:
        [[nodiscard]] GaussianManifold generate(const GeneratorConfig& config) const;
    };
}