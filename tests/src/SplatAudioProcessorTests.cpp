//
// Created by Yohanes Turnip on 2026-07-30.
//
#include <juce_core/juce_core.h>
#include "ags/engine/SplatAudioProcessor.h"
#include "ags/engine/SpiralDelayProcessor.h"
#include "ags/params/EffectParameter.h"
#include "ags/params/GMMBinding.h"
#include "ags/manifold/GaussianSplat.h"

using namespace ags::engine;
using namespace ags::params;

namespace
{
    // Builds a minimal splat with a chosen density and a default normal
    // facing the listener (so occlusion gain stays at 1.0 by default)
    ags::manifold::GaussianSplat makeTestSplat(float density)
    {
        ags::manifold::GaussianSplat splat;
        splat.density = density;
        splat.normal = { 0.0f, 0.0f, 1.0f };
        return splat;
    }
}

class SplatAudioProcessorTests final : public juce::UnitTest
{
public:
    SplatAudioProcessorTests() : juce::UnitTest("SplatAudioProcessor",
        "ags-engine") {}

    void runTest() override
    {
        beginTest("Processor with no effects and no slots passes audio through,"
                  "scaled only by the occlusion");
        {
            SplatAudioProcessor p;
            p.setSampleRate(44100.0f);

            auto splat = makeTestSplat(0.5f);
            p.updateParametersForBlock(splat);

            const float result = p.processSample(1.0f, splat);
            expectEquals(result, 1.0f);
        }

        beginTest("Density-bound wet parameter changes delay output between low and high density splats");
        {
            SplatAudioProcessor p;
            p.setSampleRate(44100.0f);

            const auto delayIndex = p.addEffect(std::make_unique<SpiralDelayProcessor>());

            p.addParameterSlot(
                delayIndex,
                2,
                std::make_unique<EffectParameter>(0.0f, 1.0f, 0.0f),
                GMMBinding{ GMMAttribute::Density, false }
                );

            auto lowDensitySplat = makeTestSplat(0.0f);
            auto highDensitySplat = makeTestSplat(1.0f);

            p.updateParametersForBlock(lowDensitySplat);
            const float lowDensityOutput = p.processSample(1.0f, lowDensitySplat);

            p.reset();

            p.updateParametersForBlock(highDensitySplat);
            const float highDensityOutput = p.processSample(1.0f, highDensitySplat);

            // Wet = 0 should return dry input unchanged; wet = 1 blends in delayed
            // (initially silent) signal, so output should drop towards 0
            expectEquals(lowDensityOutput, 1.0f);
            expectLessThan(highDensityOutput, lowDensityOutput);
        }

        beginTest("Occlusion gain scales output based on rotated splat normal");
        {
            SplatAudioProcessor p;
            p.setSampleRate(44100.0f);

            auto facingSplat = makeTestSplat(0.5f);
            facingSplat.normal = { 0.0f, 0.0f, 1.0f };

            auto occludedSplat = makeTestSplat(0.5f);
            occludedSplat.normal = { 0.0f, 0.0f, -1.0f };

            p.updateParametersForBlock(facingSplat);
            const float facingOutput = p.processSample(1.0f, facingSplat);

            p.updateParametersForBlock(occludedSplat);
            const float occludedOutput = p.processSample(1.0f, occludedSplat);

            expectLessThan(occludedOutput, facingOutput);
        }

        beginTest("Multiple parameter slots on the same effect both apply correctly");
        {
            SplatAudioProcessor p;
            p.setSampleRate(44100.0f);

            const auto delayIndex = p.addEffect(std::make_unique<SpiralDelayProcessor>());

            p.addParameterSlot(
                delayIndex,
                0,
                std::make_unique<EffectParameter>(1.0f, 1000.0f, 100.0f),
                GMMBinding{ GMMAttribute::Eccentricity, false }
                );

            p.addParameterSlot(
                delayIndex,
                1,
                std::make_unique<EffectParameter>(0.0f, 0.99f, 0.5f),
                GMMBinding{ GMMAttribute::Curvature, false }
                );

            auto splat = makeTestSplat(0.3f);
            splat.eccentricity = 5.0f;
            splat.curvature = 0.4f;

            // Should not throw or crash with two active slots targeting
            // the same effect index but different paramIds
            p.updateParametersForBlock(splat);
            const float result = p.processSample(1.0f, splat);

            expect(std::isfinite(result));
        }
    }
};

static SplatAudioProcessorTests splatAudioProcessorTests;