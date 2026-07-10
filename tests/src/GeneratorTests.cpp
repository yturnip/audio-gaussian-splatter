//
// Created by Yohanes Turnip on 2026-07-08.
//

#include <juce_core/juce_core.h>
#include <ags/manifold/Generators/SphereBranchingGenerator.h>
#include <ags/manifold/Generators/DomeBranchingGenerator.h>

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

class GeneratorTests : public juce::UnitTest
{
public:
    GeneratorTests() : juce::UnitTest("GeneratorTests") {}

    void runTest() override
    {
        GeneratorConfig config;
        config.numHubs = 8;
        config.pointsPerHub = 4;
        config.spread = 0.1f;
        config.seed = 1234;

        beginTest("Sphere is deterministic");
        {
            SphereBranchingGenerator gen;
            auto a = gen.generate(config);
            auto b = gen.generate(config);

            expectEquals((int) a.size(), (int) b.size());
            for (size_t i = 0; i < a.size(); ++i)
            {
                expectVecNear(*this, a.splats()[i].position, b.splats()[i].position, 1.0e-6f);
                expectWithinAbsoluteError(a.splats()[i].density, b.splats()[i].density, 1.0e-6f);
                expectWithinAbsoluteError(a.splats()[i].eccentricity, b.splats()[i].eccentricity, 1.0e-6f);
                expectWithinAbsoluteError(a.splats()[i].shColor, b.splats()[i].shColor, 1.0e-6f);
                expectWithinAbsoluteError(a.splats()[i].curvature, b.splats()[i].curvature, 1.0e-6f);
            }
        }

        beginTest("Sphere size and ranges");
        {
            SphereBranchingGenerator gen;
            auto m = gen.generate(config);

            expectEquals((int) m.size(), config.numHubs * config.pointsPerHub);

            for (const auto& s : m.splats())
            {
                expectWithinAbsoluteError(glm::length(s.position), 1.0f, 1.0e-4f);
                expectWithinAbsoluteError(glm::length(s.normal), 1.0f, 1.0e-4f);
                expect(s.density > 0.0f && s.density <= 1.0f);
                expect(s.shColor >= 0.0f && s.shColor <= 1.0f);
                expect(s.eccentricity >= 1.0f);
            }
        }

        beginTest("Dome size and hemisphere constraint");
        {
            DomeBranchingGenerator gen;
            auto m = gen.generate(config);

            expectEquals((int) m.size(), config.numHubs * config.pointsPerHub);

            for (const auto& s : m.splats())
            {
                expect(s.position.y >= -1.0e-6f);
                expectWithinAbsoluteError(glm::length(s.position), 1.0f, 1.0e-4f);
                expectWithinAbsoluteError(glm::length(s.normal), 1.0f, 1.0e-4f);
                expect(s.density > 0.0f && s.density <= 1.0f);
                expect(s.shColor >= 0.0f && s.shColor <= 1.0f);
                expect(s.eccentricity >= 1.0f);
            }
        }
    }
};

static GeneratorTests generatorTests;