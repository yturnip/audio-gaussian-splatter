#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "ags/manifold/GaussianManifold.h"
#include "ags/manifold/ManifoldRotator.h"
#include "ags/manifold/Generators/SphereBranchingGenerator.h"
#include "ags/manifold/Generators/DomeBranchingGenerator.h"
#include "ags/manifold/Generators/GeneratorConfig.h"
#include "ags/engine/AudioEngine.h"
#include "ags/params/GMMBinding.h"

class AgsAudioProcessor : public juce::AudioProcessor
{
public:
    AgsAudioProcessor();
    ~AgsAudioProcessor() override = default;

    void prepareToPlay(double, int) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Audio Gaussian Splatter"; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override
    {
        return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
    }

    [[nodiscard]] const ags::manifold::GaussianManifold& getManifold() const { return manifold; }

    [[nodiscard]] ags::manifold::GaussianManifold getRotatedManifold() const
    {
        ags::manifold::ManifoldRotator rotator;
        ags::manifold::RotationAngles angles;
        angles.yawRadians = rotationYaw->get();
        angles.pitchRadians = rotationPitch->get();
        return rotator.rotate(manifold, angles);
    }

    [[nodiscard]] ags::engine::SplatAudioProcessor& getSplatProcessor(size_t index)
    {
        return audioEngine.getSplatProcessor(index);
    }

    [[nodiscard]] ags::engine::AudioEngine& getAudioEngine() { return audioEngine; }

    juce::AudioParameterFloat* rotationYaw { nullptr };
    juce::AudioParameterFloat* rotationPitch { nullptr };

private:
    ags::manifold::GaussianManifold manifold;
    ags::engine::AudioEngine audioEngine;

    juce::AudioBuffer<float> perSplatBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AgsAudioProcessor)
};
