//
// Created by Yohanes Turnip on 2026-07-17.
//

#ifndef AUDIOGAUSSIANSPLATTER_GMMBINDING_H
#define AUDIOGAUSSIANSPLATTER_GMMBINDING_H

namespace ags::params
{
    enum class GMMAttribute
    {
        None,
        Density,
        ShColor,
        Eccentricity,
        Curvature
    };

    struct GMMBinding
    {
        GMMAttribute attribute { GMMAttribute::None };
        bool invert { false };

        [[nodiscard]] bool isActive() const { return attribute != GMMAttribute::None; }
    };
}
#endif //AUDIOGAUSSIANSPLATTER_GMMBINDING_H
