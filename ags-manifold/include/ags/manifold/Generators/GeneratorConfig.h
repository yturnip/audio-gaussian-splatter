//
// Created by Yohanes Turnip on 2026-07-02.
//

#ifndef AUDIOGAUSSIANSPLATTER_GENERATORCONFIG_H
#define AUDIOGAUSSIANSPLATTER_GENERATORCONFIG_H
#include <atomic>

#endif //AUDIOGAUSSIANSPLATTER_GENERATORCONFIG_H

namespace ags::manifold
{
    struct GeneratorConfig
    {
        int numHubs { 64 };
        int pointsPerHub { 10 };
        float spread { 0.4f };
        std::uint32_t seed { 0 };
    };
}