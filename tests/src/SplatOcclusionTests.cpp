//
// Created by Yohanes Turnip on 2026-07-17.
//
#include <juce_core/juce_core.h>
#include "ags/params/SplatOcclusion.h"
#include "ags/manifold/GaussianSplat.h"

using namespace ags::params;

class SplatOcclusionTests : public juce::UnitTest
{
public:
    SplatOcclusionTests() : juce::UnitTest("SplatOcclusionTests") {}

    void runTest() override
    {
        beginTest("Occlusion clamps below minimum");
        {
            ags::manifold::GaussianSplat splat;
            splat.normal = {0.0f, 0.0f, -0.5f};
            expectWithinAbsoluteError(SplatOcclusion::compute(splat), 0.3f, 1.0e-6f);
        }

        beginTest("Occlusion clamps above maximum");
        {
            ags::manifold::GaussianSplat splat;
            splat.normal = {0.0f, 0.0f, 1.5f};
            expectWithinAbsoluteError(SplatOcclusion::compute(splat), 1.0f, 1.0e-6f);
        }

        beginTest("Occlusion passes through mid range values unchanged");
        {
            ags::manifold::GaussianSplat splat;
            splat.normal = {0.0f, 0.0f, 0.5f};
            expectWithinAbsoluteError(SplatOcclusion::compute(splat), 0.5f, 1.0e-6f);
        }
    }
};

[[maybe_unused]] static SplatOcclusionTests splatOcclusionTests;