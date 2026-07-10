//
// Created by Yohanes Turnip on 2026-07-02.
//

#pragma once
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

namespace ags::manifold
{
    struct GaussianSplat
    {
        glm::vec3 position {0.0f, 0.0f, 0.0f};
        float density {0.0f};
        float eccentricity {0.0f};
        float shColor {0.0f};
        float curvature {0.0f};
        glm::vec3 normal {0.0f, 1.0f, 0.0f};
    };
}