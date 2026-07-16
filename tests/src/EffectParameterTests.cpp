//
// Created by Yohanes Turnip on 2026-07-16.
//
#include <juce_core/juce_core.h>
#include <ags/params/EffectParameter.h>

using namespace ags::params;

class EffectParameterTests : public juce::UnitTest
{
public:
    EffectParameterTests() : juce::UnitTest("EffectParameterTests") {}

    void runTest() override
    {
        beginTest("Default value is clamped and returned when unbound");
        {
            EffectParameter param(0.0f, 1.0f, 0.8f);
            expectWithinAbsoluteError(param.getValue(), 0.8f, 1.0e-6f);
            expect(!param.isGMMBound());
        }

        beginTest("Constructor clamps out-of-range default");
        {
            EffectParameter param(0.0f, 1.0f, 5.0f);
            expectWithinAbsoluteError(param.getValue(), 1.0f, 1.0e-6f);
        }

        beginTest("setManualValue clamps to range");
        {
            EffectParameter param(20.0f, 20000.0f, 1000.0f);

            param.setManualValue(500.0f);
            expectWithinAbsoluteError(param.getValue(), 500.0f, 1.0e-3f);

            param.setManualValue(-10.0f);
            expectWithinAbsoluteError(param.getValue(), 20.0f, 1.0e-3f);

            param.setManualValue(999999.0f);
            expectWithinAbsoluteError(param.getValue(), 20000.0f, 1.0e-3f);
        }

        beginTest("setGMMDrivenValue scales 0..1 into [min, max] and overrides manual value");
        {
            EffectParameter param(20.0f, 20000.0f, 1000.0f);

            param.setGMMDrivenValue(0.0f);
            expectWithinAbsoluteError(param.getValue(), 20.0f, 1.0e-3f);
            expect(param.isGMMBound());

            param.setGMMDrivenValue(1.0f);
            expectWithinAbsoluteError(param.getValue(), 20000.0f, 1.0e-3f);

            param.setGMMDrivenValue(0.5f);
            expectWithinAbsoluteError(param.getValue(), 10010.0f, 1.0e-3f);
        }

        beginTest("setGMMDrivenValue clamps normalized input outside 0..1");
        {
            EffectParameter param(0.0f, 1.0f, 0.5f);

            param.setGMMDrivenValue(-0.5f);
            expectWithinAbsoluteError(param.getValue(), 0.0f, 1.0e-6f);

            param.setGMMDrivenValue(1.5f);
            expectWithinAbsoluteError(param.getValue(), 1.0f, 1.0e-6f);
        }

        beginTest("clearGMMBinding reverts to manual value");
        {
            EffectParameter param(0.0f, 1.0f, 0.3f);

            param.setGMMDrivenValue(0.9f);
            expectWithinAbsoluteError(param.getValue(), 0.9f, 1.0e-6f);

            param.clearGMMBinding();
            expectWithinAbsoluteError(param.getValue(), 0.3f, 1.0e-6f);
            expect(!param.isGMMBound());
        }

        beginTest("Different parameters maintain independent ranges");
        {
            EffectParameter gain(0.0f, 1.0f, 0.8f);
            EffectParameter cutoff(20.0f, 20000.0f, 1000.0f);

            gain.setGMMDrivenValue(0.6f);
            cutoff.setGMMDrivenValue(0.6f);

            expectWithinAbsoluteError(gain.getValue(), 0.6f, 1.0e-6f);
            expectWithinAbsoluteError(cutoff.getValue(), 12008.0f, 1.0e-3f);
        }
    }
};

[[maybe_unused]] static EffectParameterTests effectParameterTests;
