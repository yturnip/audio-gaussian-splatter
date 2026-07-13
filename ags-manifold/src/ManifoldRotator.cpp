//
// Created by Yohanes Turnip on 2026-07-09.
//

#include "ags/manifold/ManifoldRotator.h"

#include <glm/mat3x3.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ags::manifold
{
    GaussianManifold ManifoldRotator::rotate(const GaussianManifold& manifold,
        const RotationAngles& angles) const
    {
        glm::mat4 rotation4 {1.0f};
        rotation4 = glm::rotate(rotation4, angles.yawRadians, glm::vec3(0.0f, 1.0f, 0.0f));
        rotation4 = glm::rotate(rotation4, angles.pitchRadians, glm::vec3(1.0f, 0.0f, 0.0f));
        rotation4 = glm::rotate(rotation4, angles.rollRadians, glm::vec3(0.0f, 0.0f, 1.0f));

        const glm::mat3 rotation3 {rotation4};

        GaussianManifold::Container rotatedSplats;
        rotatedSplats.reserve(manifold.size());

        for (const auto& splat : manifold.splats())
        {
            GaussianSplat rotated = splat;
            rotated.position = rotation3 * splat.position;
            rotated.normal = glm::normalize(rotation3 * splat.normal);

            rotatedSplats.push_back(rotated);
        }

        return GaussianManifold {std::move(rotatedSplats)};
    }
}