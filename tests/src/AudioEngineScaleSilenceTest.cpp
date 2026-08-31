//
// Created by Yohanes Turnip on 2026-08-30.
//

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

#include "ags/engine/AudioEngine.h"
#include "ags/engine/EffectRegistry.h"
#include "ags/manifold/GaussianManifold.h"
#include "ags/manifold/Generators/SphereBranchingGenerator.h"
#include "ags/manifold/Generators/GeneratorConfig.h"

using namespace ags::engine;
using namespace ags::manifold;

namespace
{
    constexpr double kSampleRate = 44100.0;
    constexpr int kBlockSize = 512;

    GaussianManifold makeManifold(int targetSplatCount)
    {
        GeneratorConfig config;
        config.pointsPerHub = 20;
        config.numHubs = std::max(1, targetSplatCount / (1 + config.pointsPerHub));

        SphereBranchingGenerator generator;
        return generator.generate(config);
    }

    // One SplatAudioProcessor per splat, registered in manifold order, same
    // as AgsAudioProcessor's constructor. Effects are added afterward via
    // AudioEngine::addEffectForAll, the identical entry point the live GUI's
    // "+" button uses, because addEffectForAll's per-splat loop is itself
    // one of the suspected contributors to the scaling bottleneck.
    void buildEngine(AudioEngine& engine, int splatCount)
    {
        for (int i = 0; i < splatCount; ++i)
            engine.addSplatProcessor(std::make_unique<SplatAudioProcessor>());

        engine.setSampleRate(static_cast<float>(kSampleRate));
        engine.reset();
    }

    // Runs numBlocks blocks of block-based processing through the engine and
    // reports whether output ever goes non-finite or stays at exact zero for
    // an entire block (the two observable symptoms of "glitch" vs.
    // "permanent silence" respectively). wentPermanentlySilentAfterNonZero
    // is only set once a full-zero block is seen AFTER at least one
    // non-zero block already occurred - the exact signature of the reported
    // bug (audio works, then permanently stops - not "never started").
    struct RunResult
    {
        bool everProducedNonZero = false;
        bool wentPermanentlySilentAfterNonZero = false;
        bool sawNonFinite = false;
        int firstSilentBlockIndex = -1;
    };

    RunResult driveBlocks(AudioEngine& engine, const GaussianManifold& manifold,
                           int numChannelsOut, int numBlocks)
    {
        RunResult result;

        std::vector<float> inputBlock(static_cast<size_t>(kBlockSize));
        for (int i = 0; i < kBlockSize; ++i)
            inputBlock[static_cast<size_t>(i)] = std::sin(2.0f * juce::MathConstants<float>::pi
                * 440.0f * static_cast<float>(i) / static_cast<float>(kSampleRate));

        juce::AudioBuffer<float> perSplatBuffer(numChannelsOut, kBlockSize);

        bool sawNonZeroBlockAlready = false;

        for (int block = 0; block < numBlocks; ++block)
        {
            engine.processBlock(inputBlock.data(), kBlockSize, manifold, perSplatBuffer);

            float sumAbs = 0.0f;
            bool nonFiniteThisBlock = false;

            for (int ch = 0; ch < perSplatBuffer.getNumChannels(); ++ch)
            {
                const auto* channelData = perSplatBuffer.getReadPointer(ch);
                for (int i = 0; i < kBlockSize; ++i)
                {
                    const float sample = channelData[i];
                    if (!std::isfinite(sample))
                        nonFiniteThisBlock = true;
                    sumAbs += std::abs(sample);
                }
            }

            if (nonFiniteThisBlock)
                result.sawNonFinite = true;

            const bool blockIsSilent = (sumAbs == 0.0f);

            if (!blockIsSilent)
            {
                result.everProducedNonZero = true;
                sawNonZeroBlockAlready = true;
            }
            else if (sawNonZeroBlockAlready && result.firstSilentBlockIndex < 0)
            {
                result.firstSilentBlockIndex = block;
                result.wentPermanentlySilentAfterNonZero = true;
            }
        }

        return result;
    }

    // Repeats driveBlocks with increasing effect counts on the same engine,
    // matching the live repro's "add effects one at a time" pattern, so this
    // test can report the exact effect count at which the silence wall
    // appears for a given splat count - directly comparable to the
    // migration doc's "hard wall at 5 effects" (800 splats) finding.
    void runScalingSweep(int splatCount, int maxEffectsToAdd, int blocksPerStage)
    {
        std::printf("\n=== splatCount=%d ===\n", splatCount);

        AudioEngine engine;
        buildEngine(engine, splatCount);

        const auto manifold = makeManifold(splatCount);
        const auto actualSplatCount = static_cast<int>(manifold.size());
        std::printf("  (actual splat count from generator: %d)\n", actualSplatCount);

        const auto& registry = EffectRegistry::all();
        jassert(!registry.empty());

        for (int effectsAdded = 1; effectsAdded <= maxEffectsToAdd; ++effectsAdded)
        {
            const auto& entry = registry[static_cast<size_t>((effectsAdded - 1) % registry.size())];
            engine.addEffectForAll(entry.create);

            const auto result = driveBlocks(engine, manifold, actualSplatCount, blocksPerStage);

            std::printf(
                "  effects=%2d  everNonZero=%s  wentSilentAfterNonZero=%s  "
                "firstSilentBlock=%d  nonFinite=%s\n",
                effectsAdded,
                result.everProducedNonZero ? "yes" : "no",
                result.wentPermanentlySilentAfterNonZero ? "YES <-- BUG" : "no",
                result.firstSilentBlockIndex,
                result.sawNonFinite ? "YES" : "no");

            if (result.wentPermanentlySilentAfterNonZero)
            {
                std::printf(
                    "  --> Repro achieved at splatCount=%d, effects=%d. "
                    "Stopping sweep for this splat count.\n",
                    splatCount, effectsAdded);
                break;
            }
        }
    }
}

// Plain main(), no JUCE application/message-manager wrapper - per migration
// doc, the user builds the VST3 and runs it through AudioPluginHost, so
// DBG()/juce Logger output is not visible during manual testing. A
// standalone console executable's std::printf output IS visible, matching
// the approach that already worked for EffectChainSilenceTest.cpp this
// session. No ScopedJuceInitialiser_GUI here (see header comment) - this
// path never touches juce_events/juce_gui_basics.
int main(int argc, char** argv)
{
    // Defaults chosen to bracket the migration doc's three observed data
    // points (80 splats = no limit found, 500 = glitchy up to ~10, 800 =
    // hard wall at 5) without needing an actual VST3 host. Override via
    // command-line args for a quicker/narrower run:
    //   audio-engine-scale-silence-test <maxEffects> <blocksPerStage> <splatCounts...>
    int maxEffectsToAdd = 8;
    int blocksPerStage = 50;
    std::vector<int> splatCounts { 80, 500, 800 };

    if (argc > 1) maxEffectsToAdd = std::atoi(argv[1]);
    if (argc > 2) blocksPerStage = std::atoi(argv[2]);
    if (argc > 3)
    {
        splatCounts.clear();
        for (int i = 3; i < argc; ++i)
            splatCounts.push_back(std::atoi(argv[i]));
    }

    std::printf("AudioEngineScaleSilenceTest: sampleRate=%.0f blockSize=%d "
                "maxEffects=%d blocksPerStage=%d\n",
                kSampleRate, kBlockSize, maxEffectsToAdd, blocksPerStage);

    for (const int splatCount : splatCounts)
        runScalingSweep(splatCount, maxEffectsToAdd, blocksPerStage);

    std::printf("\nDone. Compare firstSilentBlock/effects-at-onset across "
                "splat counts against MIGRATION_CONTEXT_v2.md's reported "
                "80/500/800-splat data points before drawing conclusions.\n");

    return 0;
}