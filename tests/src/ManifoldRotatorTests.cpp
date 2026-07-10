//
// Created by Yohanes Turnip on 2026-07-09.
//
#include <juce_core/juce_core.h>
#include <glm/glm.hpp>
#include "ags/manifold/ManifoldRotator.h"
#include "ags/manifold/Generators/SphereBranchingGenerator.h"
#include "ags/manifold/Generators/DomeBranchingGenerator.h"

using namespace ags::manifold;

namespace
{
    void expectVecNear (juce::UnitTest& t, const glm::vec3& a, const glm::vec3& b, float eps)
    {
        t.expectWithinAbsoluteError(a.x, b.x, eps);
        t.expectWithinAbsoluteError(a.y, b.y, eps);
        t.expectWithinAbsoluteError(a.z, b.z, eps);
    }
}

class ManifoldRotatorTests : public juce::UnitTest
{
public:
    ManifoldRotatorTests() : juce::UnitTest("ManifoldRotatorTests") {}

    void runTest() override
    {
        beginTest("Sphere: identity rotation leaves manifold unchanged");
        {
            GeneratorConfig config;
            config.numHubs = 4;
            config.pointsPerHub = 2;
            config.spread = 0.1f;
            config.seed = 42;

            SphereBranchingGenerator gen;
            auto original = gen.generate(config);

            ManifoldRotator rotator;
            RotationAngles identity {};
            auto rotated = rotator.rotate(original, identity);

            expectEquals(static_cast<int>(rotated.size()), static_cast<int>(original.size()));

            for (size_t i = 0; i < original.size(); ++i)
            {
                expectVecNear(*this, rotated.splats()[i].position, original.splats()[i].position, 1.0e-5f);
                expectVecNear(*this, rotated.splats()[i].normal, original.splats()[i].normal, 1.0e-5f);
            }
        }

        beginTest("Sphere: 90 degree yaw rotates X axis onto -Z axis");
        {
            GaussianManifold manifold;
            GaussianSplat splat;
            splat.position = { 1.0f, 0.0f, 0.0f };
            splat.normal   = { 1.0f, 0.0f, 0.0f };
            splat.density = 0.5f;
            splat.eccentricity = 1.2f;
            splat.shColor = 0.3f;
            splat.curvature = 0.7f;
            manifold.addSplat(splat);

            ManifoldRotator rotator;
            RotationAngles yaw90 { juce::MathConstants<float>::halfPi, 0.0f, 0.0f };
            auto rotated = rotator.rotate(manifold, yaw90);

            expectVecNear(*this, rotated.splats()[0].position, { 0.0f, 0.0f, -1.0f }, 1.0e-4f);
            expectVecNear(*this, rotated.splats()[0].normal,   { 0.0f, 0.0f, -1.0f }, 1.0e-4f);
        }

        beginTest("Sphere: rotation preserves vector lengths");
        {
            GeneratorConfig config;
            config.numHubs = 6;
            config.pointsPerHub = 3;
            config.spread = 0.15f;
            config.seed = 7;

            SphereBranchingGenerator gen;
            auto manifold = gen.generate(config);

            ManifoldRotator rotator;
            RotationAngles angles { 0.6f, 0.3f, 1.1f };
            auto rotated = rotator.rotate(manifold, angles);

            for (size_t i = 0; i < manifold.size(); ++i)
            {
                expectWithinAbsoluteError(glm::length(rotated.splats()[i].position),
                                           glm::length(manifold.splats()[i].position), 1.0e-4f);
                expectWithinAbsoluteError(glm::length(rotated.splats()[i].normal), 1.0f, 1.0e-4f);
            }
        }

        beginTest("Sphere: rotation preserves scalar attributes");
        {
            GeneratorConfig config;
            config.numHubs = 4;
            config.pointsPerHub = 2;
            config.spread = 0.1f;
            config.seed = 99;

            SphereBranchingGenerator gen;
            auto manifold = gen.generate(config);

            ManifoldRotator rotator;
            RotationAngles angles { 0.4f, 0.2f, 0.9f };
            auto rotated = rotator.rotate(manifold, angles);

            for (size_t i = 0; i < manifold.size(); ++i)
            {
                expectWithinAbsoluteError(rotated.splats()[i].density, manifold.splats()[i].density, 1.0e-6f);
                expectWithinAbsoluteError(rotated.splats()[i].eccentricity, manifold.splats()[i].eccentricity, 1.0e-6f);
                expectWithinAbsoluteError(rotated.splats()[i].shColor, manifold.splats()[i].shColor, 1.0e-6f);
                expectWithinAbsoluteError(rotated.splats()[i].curvature, manifold.splats()[i].curvature, 1.0e-6f);
            }
        }

        beginTest("Dome: rotation preserves vector lengths");
        {
            GeneratorConfig config;
            config.numHubs = 6;
            config.pointsPerHub = 3;
            config.spread = 0.15f;
            config.seed = 7;

            DomeBranchingGenerator gen;
            auto manifold = gen.generate(config);

            ManifoldRotator rotator;
            RotationAngles angles { 0.6f, 0.3f, 1.1f };
            auto rotated = rotator.rotate(manifold, angles);

            for (size_t i = 0; i < manifold.size(); ++i)
            {
                expectWithinAbsoluteError(glm::length(rotated.splats()[i].position),
                                           glm::length(manifold.splats()[i].position), 1.0e-4f);
                expectWithinAbsoluteError(glm::length(rotated.splats()[i].normal), 1.0f, 1.0e-4f);
            }
        }

        beginTest("Dome: rotation preserves scalar attributes");
        {
            GeneratorConfig config;
            config.numHubs = 4;
            config.pointsPerHub = 2;
            config.spread = 0.1f;
            config.seed = 99;

            DomeBranchingGenerator gen;
            auto manifold = gen.generate(config);

            ManifoldRotator rotator;
            RotationAngles angles { 0.4f, 0.2f, 0.9f };
            auto rotated = rotator.rotate(manifold, angles);

            for (size_t i = 0; i < manifold.size(); ++i)
            {
                expectWithinAbsoluteError(rotated.splats()[i].density, manifold.splats()[i].density, 1.0e-6f);
                expectWithinAbsoluteError(rotated.splats()[i].eccentricity, manifold.splats()[i].eccentricity, 1.0e-6f);
                expectWithinAbsoluteError(rotated.splats()[i].shColor, manifold.splats()[i].shColor, 1.0e-6f);
                expectWithinAbsoluteError(rotated.splats()[i].curvature, manifold.splats()[i].curvature, 1.0e-6f);
            }
        }
    }
};

[[maybe_unused]] static ManifoldRotatorTests manifoldRotatorTests;