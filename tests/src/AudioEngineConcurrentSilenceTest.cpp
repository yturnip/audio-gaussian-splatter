//
// Created by Yohanes Turnip on 2026-08-30.
//

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <thread>
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
using Clock = std::chrono::steady_clock;

namespace
{
    constexpr double kSampleRate = 44100.0;
    constexpr int kBlockSize = 512;
    constexpr double kBlockBudgetMs = 1000.0 * static_cast<double>(kBlockSize) / kSampleRate;

    GaussianManifold makeManifold(int targetSplatCount)
    {
        GeneratorConfig config;
        config.pointsPerHub = 20;
        config.numHubs = std::max(1, targetSplatCount / (1 + config.pointsPerHub));

        SphereBranchingGenerator generator;
        return generator.generate(config);
    }

    void buildEngine(AudioEngine& engine, int splatCount)
    {
        for (int i = 0; i < splatCount; ++i)
            engine.addSplatProcessor(std::make_unique<SplatAudioProcessor>());

        engine.setSampleRate(static_cast<float>(kSampleRate));
        engine.reset();
    }

    struct SharedState
    {
        std::atomic<bool> stopRequested { false };
        std::atomic<bool> effectAddInProgress { false };
        std::atomic<int> effectsAddedSoFar { 0 };

        std::atomic<bool> everProducedNonZero { false };
        std::atomic<bool> wentSilentAfterNonZero { false };
        std::atomic<bool> sawNonFinite { false };
        std::atomic<int> firstSilentBlockIndex { -1 };
        std::atomic<int> lateBlockCount { 0 };
        std::atomic<double> worstBlockMs { 0.0 };
        std::atomic<double> totalBlockMs { 0.0 };
        std::atomic<int> blocksProcessed { 0 };
        std::atomic<bool> firstSilentBlockRacedWithAdd { false };
    };

    // Calls the REAL AudioEngine::processBlock() directly - this is the
    // whole point of v3. No reimplementation of its internals here.
    void audioThreadFunc(AudioEngine& engine, const GaussianManifold& manifold,
                          int numChannelsOut, int numBlocksToRun, SharedState& shared)
    {
        std::vector<float> inputBlock(static_cast<size_t>(kBlockSize));
        for (int i = 0; i < kBlockSize; ++i)
            inputBlock[static_cast<size_t>(i)] = std::sin(2.0f * juce::MathConstants<float>::pi
                * 440.0f * static_cast<float>(i) / static_cast<float>(kSampleRate));

        juce::AudioBuffer<float> perSplatBuffer(numChannelsOut, kBlockSize);

        bool sawNonZeroBlockAlready = false;
        const auto blockPeriod = std::chrono::duration<double, std::milli>(kBlockBudgetMs);

        for (int block = 0; block < numBlocksToRun && !shared.stopRequested.load(); ++block)
        {
            const auto callStart = Clock::now();
            const bool addWasInFlightAtStart = shared.effectAddInProgress.load();

            engine.processBlock(inputBlock.data(), kBlockSize, manifold, perSplatBuffer);

            const auto callEnd = Clock::now();
            const double elapsedMs = std::chrono::duration<double, std::milli>(callEnd - callStart).count();

            double prevTotal = shared.totalBlockMs.load();
            while (!shared.totalBlockMs.compare_exchange_weak(prevTotal, prevTotal + elapsedMs)) {}

            if (elapsedMs > kBlockBudgetMs)
                shared.lateBlockCount.fetch_add(1);

            double prevWorst = shared.worstBlockMs.load();
            while (elapsedMs > prevWorst &&
                   !shared.worstBlockMs.compare_exchange_weak(prevWorst, elapsedMs)) {}

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
                shared.sawNonFinite.store(true);

            const bool blockIsSilent = (sumAbs == 0.0f);
            const bool addInFlightNow = addWasInFlightAtStart || shared.effectAddInProgress.load();

            if (!blockIsSilent)
            {
                shared.everProducedNonZero.store(true);
                sawNonZeroBlockAlready = true;
            }
            else if (sawNonZeroBlockAlready && shared.firstSilentBlockIndex.load() < 0)
            {
                shared.firstSilentBlockIndex.store(block);
                shared.wentSilentAfterNonZero.store(true);
                shared.firstSilentBlockRacedWithAdd.store(addInFlightNow);
            }

            shared.blocksProcessed.fetch_add(1);

            const auto elapsed = Clock::now() - callStart;
            const auto sleepFor = blockPeriod - elapsed;
            if (sleepFor > std::chrono::duration<double, std::milli>(0))
                std::this_thread::sleep_for(sleepFor);
        }
    }

    void guiThreadFunc(AudioEngine& engine, int maxEffectsToAdd,
                        std::chrono::milliseconds delayBetweenAdds, SharedState& shared)
    {
        const auto& registry = EffectRegistry::all();
        jassert(!registry.empty());

        for (int i = 0; i < maxEffectsToAdd && !shared.stopRequested.load(); ++i)
        {
            std::this_thread::sleep_for(delayBetweenAdds);

            shared.effectAddInProgress.store(true);
            const auto& entry = registry[static_cast<size_t>(i % registry.size())];
            engine.addEffectForAll(entry.create);
            shared.effectAddInProgress.store(false);

            shared.effectsAddedSoFar.fetch_add(1);
        }
    }

    void runConcurrentTrial(int splatCount, int maxEffectsToAdd, int totalBlocksToRun)
    {
        std::printf("\n=== CONCURRENT TRIAL (v3, real processBlock): splatCount=%d, maxEffects=%d ===\n",
                    splatCount, maxEffectsToAdd);

        AudioEngine engine;
        buildEngine(engine, splatCount);

        const auto manifold = makeManifold(splatCount);
        const auto actualSplatCount = static_cast<int>(manifold.size());
        std::printf("  (actual splat count from generator: %d, block budget: %.3f ms, "
                    "hardware CPUs reported: %d)\n",
                    actualSplatCount, kBlockBudgetMs, juce::SystemStats::getNumCpus());

        SharedState shared;

        const auto totalRunEstimateMs = totalBlocksToRun * kBlockBudgetMs;
        const auto delayBetweenAdds = std::chrono::milliseconds(
            static_cast<long>(totalRunEstimateMs / std::max(1, maxEffectsToAdd + 1)));

        std::thread audioThread(audioThreadFunc, std::ref(engine), std::cref(manifold),
                                 actualSplatCount, totalBlocksToRun, std::ref(shared));
        std::thread guiThread(guiThreadFunc, std::ref(engine), maxEffectsToAdd,
                              delayBetweenAdds, std::ref(shared));

        guiThread.join();
        audioThread.join();

        const int blocks = std::max(1, shared.blocksProcessed.load());
        const double avgBlockMs = shared.totalBlockMs.load() / blocks;

        std::printf(
            "  effectsAdded=%d  blocksProcessed=%d  everNonZero=%s  "
            "wentSilentAfterNonZero=%s  firstSilentBlock=%d  "
            "racedWithAdd=%s  nonFinite=%s\n",
            shared.effectsAddedSoFar.load(),
            shared.blocksProcessed.load(),
            shared.everProducedNonZero.load() ? "yes" : "no",
            shared.wentSilentAfterNonZero.load() ? "YES <-- BUG" : "no",
            shared.firstSilentBlockIndex.load(),
            shared.firstSilentBlockRacedWithAdd.load() ? "YES <-- race confirmed" : "no",
            shared.sawNonFinite.load() ? "YES" : "no");

        std::printf(
            "  lateBlocks=%d / %d  worstBlockMs=%.3f  avgBlockMs=%.3f  (budget=%.3f)\n",
            shared.lateBlockCount.load(), shared.blocksProcessed.load(),
            shared.worstBlockMs.load(), avgBlockMs, kBlockBudgetMs);

        if (shared.wentSilentAfterNonZero.load())
        {
            std::printf(
                "  --> Repro achieved at splatCount=%d under concurrent add+process load.\n",
                splatCount);
        }
        else if (shared.lateBlockCount.load() > 0)
        {
            std::printf(
                "  --> No silence, but %d block(s) missed the real-time deadline.\n",
                shared.lateBlockCount.load());
        }
        else
        {
            std::printf(
                "  --> No silence and no deadline misses detected at this splat/effect "
                "count under this thread-timing pattern.\n");
        }
    }
}

int main(int argc, char** argv)
{
    int maxEffectsToAdd = 8;
    int totalBlocksToRun = 400;
    std::vector<int> splatCounts { 80, 500, 800 };

    if (argc > 1) maxEffectsToAdd = std::atoi(argv[1]);
    if (argc > 2) totalBlocksToRun = std::atoi(argv[2]);
    if (argc > 3)
    {
        splatCounts.clear();
        for (int i = 3; i < argc; ++i)
            splatCounts.push_back(std::atoi(argv[i]));
    }

    std::printf("AudioEngineConcurrentSilenceTest (v3, real processBlock): sampleRate=%.0f "
                "blockSize=%d maxEffects=%d totalBlocks=%d blockBudgetMs=%.3f\n",
                kSampleRate, kBlockSize, maxEffectsToAdd, totalBlocksToRun, kBlockBudgetMs);

    for (const int splatCount : splatCounts)
        runConcurrentTrial(splatCount, maxEffectsToAdd, totalBlocksToRun);

    std::printf("\nDone. Compare avgBlockMs/worstBlockMs here against v2's 'TOTAL measured' "
                "figures at the same splat counts (60: 2.940ms, 460: 6.830-6.944ms, "
                "760: 10.395-10.522ms) - if parallelization is working, these should now "
                "be noticeably lower, roughly in proportion to (reported CPU count).\n");

    return 0;
}
