//
// Created by Yohanes Turnip on 2026-07-02.
//
#include "ags/manifold/Generators/SphereBranchingGenerator.h"
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <random>
#include <cmath>
#include <juce_core/maths/juce_MathsFunctions.h>

namespace ags::manifold
{
    GaussianManifold SphereBranchingGenerator::generate(const GeneratorConfig &config) const
    {
        GaussianManifold manifold;

        std::mt19937 rng(config.seed);
        std::normal_distribution<float> jitter(0.0f, config.spread);
        std::normal_distribution<float> logScaleDist(-1.0f, 0.5f);

        const int numHubs = config.numHubs;
        const float phi = juce::MathConstants<float>::pi * (3.0f - std::sqrt(5.0f));

        for (int i = 0; i < numHubs; ++i)
        {
            const float t = (numHubs > 1) ? (float) i / (float) (numHubs - 1) : 0.0f;
            const float y = 1.0f - 2.0f * t;
            const float radius = std::sqrt(std::max(0.0f, 1.0f - y * y));
            const float theta = phi * i;

            const float hx = radius * std::cos (theta);
            const float hy = y;
            const float hz = radius * std::sin (theta);
            const glm::vec3 hubCenter { hx, hy, hz };

            for (int p = 0; p < config.pointsPerHub; ++p)
            {
                glm::vec3 noise { jitter(rng), jitter(rng), jitter(rng) };
                glm::vec3 pos = hubCenter + noise;

                const float len = glm::length(pos);
                if (len > 1.0e-6f)
                    pos = pos / len;

                const glm::vec3 scale {
                    std::exp (logScaleDist(rng)),
                    std::exp (logScaleDist(rng)),
                    std::exp (logScaleDist(rng))
                };
                const float maxScale = std::max ({ scale.x, scale.y, scale.z });
                const float minScale = std::min ({ scale.x, scale.y, scale.z });
                const float eccentricity = maxScale / std::max (minScale, 1.0e-6f);

                const glm::vec3 normal = pos;

                const float distToHub = glm::length(pos - hubCenter);
                const float shColor = std::clamp (1.0f - (distToHub / (config.spread * 2.0f)), 0.0f, 1.0f);
                const float density = std::exp (-distToHub * 4.0f);
                const float curvature = glm::dot(pos, hubCenter);

                GaussianSplat splat;
                splat.position = pos;
                splat.normal = normal;
                splat.eccentricity = eccentricity;
                splat.density = density;
                splat.shColor = shColor;
                splat.curvature = curvature;

                manifold.addSplat(splat);
            }
        }

        return manifold;
    }
}
