//
// Created by Yohanes Turnip on 2026-07-02.
//

#ifndef AUDIOGAUSSIANSPLATTER_GENERATORCONFIG_H
#define AUDIOGAUSSIANSPLATTER_GENERATORCONFIG_H
#include <atomic>

namespace ags::manifold
{
    struct GeneratorConfig
    {
        int numHubs { 64 };
        int pointsPerHub { 20 };
        float spread { 0.1f };
        std::uint32_t seed { 0 };
    };
}

#endif //AUDIOGAUSSIANSPLATTER_GENERATORCONFIG_H