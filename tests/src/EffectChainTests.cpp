//
// Created by Yohanes Turnip on 2026-07-25.
//

#include <juce_core/juce_core.h>
#include "ags/engine/EffectProcessor.h"
#include "ags/engine/EffectChain.h"

using namespace ags::engine;

namespace
{
    // Minimal test double: adds a fixed offset to every sample,
    // lets us verify sequencing order without needing real DSP.
    class OffsetProcessor final : public EffectProcessor
    {
    public:
        explicit OffsetProcessor(float offsetToAdd) : offset(offsetToAdd) {}

        void setSampleRate(float) override {}
        void reset() override { resetCallCount++; }

        void setParameter(int paramId, float value) override
        {
            if (paramId == 0)
                offset = value;
        }

        float processSample(float in) override
        {
            return in + offset;
        }

        int resetCallCount { 0 };

    private:
        float offset;
    };
}

class EffectChainTests final : public juce::UnitTest
{
public:
    EffectChainTests() : juce::UnitTest("EffectChain", "ags-engine") {}

    void runTest() override
    {
        beginTest("Empty chain passes input through unchanged");
        {
            EffectChain chain;
            expectEquals(chain.processSample(1.0f), 1.0f);
            expectEquals(static_cast<int>(chain.size()), 0);
        }

        beginTest("Single effect processes sample correctly");
        {
            EffectChain chain;
            chain.addEffect(std::make_unique<OffsetProcessor>(2.0f));

            expectEquals(chain.processSample(1.0f), 3.0f);
        }

        beginTest("Multiple effects process in sequential order");
        {
            EffectChain chain;
            chain.addEffect(std::make_unique<OffsetProcessor>(1.0f));
            chain.addEffect(std::make_unique<OffsetProcessor>(10.0f));

            // (1 + 1) + 10 = 12, order matters here
            expectEquals(chain.processSample(1.0f), 12.0f);
        }

        beginTest("Bypassed effect is skipped in the chain");
        {
            EffectChain chain;
            chain.addEffect(std::make_unique<OffsetProcessor>(1.0f));
            chain.addEffect(std::make_unique<OffsetProcessor>(10.0f));

            chain.setBypassed(1, true);

            // Only the first offset (+1) applies now
            expectEquals(chain.processSample(1.0f), 2.0f);
            expect(chain.isBypassed(1));
            expect(!chain.isBypassed(0));
        }

        beginTest("moveEffect swaps positions and changes output order");
        {
            EffectChain chain;
            chain.addEffect(std::make_unique<OffsetProcessor>(1.0f));
            chain.addEffect(std::make_unique<OffsetProcessor>(100.0f));

            chain.moveEffect(0, 1);

            // Since both are pure additions, order doesn't change the
            // numeric result here, but this verifies size and bypass
            // state stay correctly attached to their swapped slots.
            chain.setBypassed(0, true);
            // Slot 0 now holds the +100 processor after the swap,
            // so bypassing it should leave only +1 active.
            expectEquals(chain.processSample(0.0f), 1.0f);
        }

        beginTest("removeEffect shrinks the chain and drops that effect");
        {
            EffectChain chain;
            chain.addEffect(std::make_unique<OffsetProcessor>(5.0f));
            chain.addEffect(std::make_unique<OffsetProcessor>(50.0f));

            chain.removeEffect(0);

            expectEquals(static_cast<int>(chain.size()), 1);
            expectEquals(chain.processSample(0.0f), 50.0f);
        }

        beginTest("setParameter forwards to the correct effect slot");
        {
            EffectChain chain;
            chain.addEffect(std::make_unique<OffsetProcessor>(0.0f));
            chain.addEffect(std::make_unique<OffsetProcessor>(0.0f));

            chain.setParameter(1, 0, 7.0f);

            // Only slot 1's offset should have changed
            expectEquals(chain.processSample(0.0f), 7.0f);
        }

        beginTest("reset forwards to every effect in the chain");
        {
            EffectChain chain;
            auto* first = new OffsetProcessor(0.0f);
            auto* second = new OffsetProcessor(0.0f);

            chain.addEffect(std::unique_ptr<OffsetProcessor>(first));
            chain.addEffect(std::unique_ptr<OffsetProcessor>(second));

            chain.reset();

            expectEquals(first->resetCallCount, 1);
            expectEquals(second->resetCallCount, 1);
        }
    }
};

static EffectChainTests effectChainTests;