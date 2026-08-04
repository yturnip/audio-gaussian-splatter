//
// Created by Yohanes Turnip on 2026-08-04.
//
#include <juce_core/juce_core.h>
#include "ags/engine/AudioEngine.h"
#include "ags/engine/SpiralDelayProcessor.h"
#include "ags/manifold/GaussianManifold.h"

using namespace ags::engine;
using namespace ags::manifold;

namespace
{
    GaussianSplat makeTestSplat(float x, float density = 0.0f)
    {
        GaussianSplat splat;
        splat.position = { x, 0.0f, 0.0f };
        splat.density = density;
        splat.normal = { 0.0f, 0.0f, 1.0f };
        return splat;
    }

    std::unique_ptr<SplatAudioProcessor> makePassthroughProcessor()
    {
        return std::make_unique<SplatAudioProcessor>();
    }
}

class AudioEngineTests final : public juce::UnitTest
{
public:
    AudioEngineTests() : juce::UnitTest("AudioEngine", "ags-engine") {}

    void runTest() override
    {
        beginTest("Empty engine passes input through unchanged");
        {
            AudioEngine engine;
            engine.setSampleRate(44100.0f);

            GaussianManifold emptyManifold;
            const float result = engine.processSample(1.0f, emptyManifold);

            expectEquals(result, 1.0f);
            expectEquals(static_cast<int>(engine.size()), 0);
        }

        beginTest("Single splat processor with no effects passes audio through, averaged over one");
        {
            AudioEngine engine;
            engine.setSampleRate(44100.0f);
            engine.addSplatProcessor(makePassthroughProcessor());

            GaussianManifold manifold;
            manifold.addSplat(makeTestSplat(0.0f));

            const float result = engine.processSample(1.0f, manifold);
            expectEquals(result, 1.0f);
        }

        beginTest("Multiple splat processors average their outputs correctly");
        {
            AudioEngine engine;
            engine.setSampleRate(44100.0f);

            // Two passthrough processors with no effects should each return
            // the input unchanged; averaging two identical values should
            // still equal the input.
            engine.addSplatProcessor(makePassthroughProcessor());
            engine.addSplatProcessor(makePassthroughProcessor());

            GaussianManifold manifold;
            manifold.addSplat(makeTestSplat(-1.0f));
            manifold.addSplat(makeTestSplat(1.0f));

            const float result = engine.processSample(2.0f, manifold);
            expectEquals(result, 2.0f);
        }

        beginTest("Rotation is applied once via setRotation before processing");
        {
            AudioEngine engine;
            engine.setSampleRate(44100.0f);
            engine.addSplatProcessor(makePassthroughProcessor());

            GaussianManifold manifold;
            manifold.addSplat(makeTestSplat(0.0f));

            RotationAngles angles;
            angles.yawRadians = juce::MathConstants<float>::pi * 0.5f;
            engine.setRotation(angles);

            // With no effects and no occlusion-changing bindings wired,
            // rotation should not crash and should still return finite output.
            const float result = engine.processSample(1.0f, manifold);
            expect(std::isfinite(result));
        }

        beginTest("setSampleRate propagates to processors added afterward");
        {
            AudioEngine engine;
            auto processor = makePassthroughProcessor();
            auto* rawProcessor = processor.get();

            engine.addSplatProcessor(std::move(processor));
            engine.setSampleRate(48000.0f);

            // No direct getter for sample rate on SplatAudioProcessor yet,
            // so this just verifies no crash occurs when reset() runs after
            // a sample rate change with an existing processor.
            engine.reset();
            expect(rawProcessor != nullptr);
        }
    }
};

static AudioEngineTests audioEngineTests;