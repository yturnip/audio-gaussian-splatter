//
// Created by Yohanes Turnip on 2026-07-17.
//
#include <juce_core/juce_core.h>
#include <ags/params/EffectParameter.h>
#include <ags/params/GMMBinding.h>
#include <ags/params/ParameterMapper.h>
#include <ags/manifold/GaussianSplat.h>

using namespace ags::params;

class ParameterMapperTests : public juce::UnitTest
{
public:
    ParameterMapperTests() : juce::UnitTest("ParameterMapperTests") {}

    void runTest() override
    {
        beginTest("None binding clears GMM state and reverts to manual value");
        {
            EffectParameter param(0.0f, 1.0f, 0.4f);
            param.setGMMDrivenValue(0.9f);

            ags::manifold::GaussianSplat splat;
            GMMBinding binding;
            ParameterMapper mapper;

            mapper.apply(param, binding, splat);

            expect(!param.isGMMBound());
            expectWithinAbsoluteError(param.getValue(), 0.4f, 1.0e-6f);
        }

        beginTest("ShColor binding maps directly to parameter range");
        {
            EffectParameter gain(0.0f, 1.0f, 0.5f);
            ags::manifold::GaussianSplat splat;
            splat.shColor = 0.7f;

            GMMBinding binding;
            binding.attribute = GMMAttribute::ShColor;

            ParameterMapper mapper;
            mapper.apply(gain, binding, splat);

            expect(gain.isGMMBound());
            expectWithinAbsoluteError(gain.getValue(), 0.7f, 1.0e-6f);
        }

        beginTest("Density binding scales into a wider parameter range");
        {
            EffectParameter cutoff(20.0f, 20000.0f, 1000.0f);
            ags::manifold::GaussianSplat splat;
            splat.density = 0.5f;

            GMMBinding binding;
            binding.attribute = GMMAttribute::Density;

            ParameterMapper mapper;
            mapper.apply(cutoff, binding, splat);

            expectWithinAbsoluteError(cutoff.getValue(), 10010.0f, 1.0e-3f);
        }

        beginTest("Invert flag flips normalized value before scaling");
        {
            EffectParameter gain(0.0f, 1.0f, 0.5f);
            ags::manifold::GaussianSplat splat;
            splat.shColor = 0.2f;

            GMMBinding binding;
            binding.attribute = GMMAttribute::ShColor;
            binding.invert = true;

            ParameterMapper mapper;
            mapper.apply(gain, binding, splat);

            expectWithinAbsoluteError(gain.getValue(), 0.8f, 1.0e-6f);
        }

        beginTest("Eccentricity binding clamps at soft cap");
        {
            EffectParameter gain(0.0f, 1.0f, 0.5f);
            ags::manifold::GaussianSplat splat;
            splat.eccentricity = 50.0f; // far above soft cap

            GMMBinding binding;
            binding.attribute = GMMAttribute::Eccentricity;

            ParameterMapper mapper;
            mapper.apply(gain, binding, splat);

            expectWithinAbsoluteError(gain.getValue(), 1.0f, 1.0e-6f);
        }
    }
};

[[maybe_unused]] static ParameterMapperTests parameterMapperTests;