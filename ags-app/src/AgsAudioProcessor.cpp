#include "AgsAudioProcessor.h"
#include "AgsAudioProcessorEditor.h"
#include "ags/engine/SpiralDelayProcessor.h"
#include "ags/params/EffectParameter.h"
#include "ags/params/GMMBinding.h"

AgsAudioProcessor::AgsAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    ags::manifold::GeneratorConfig config;
    ags::manifold::SphereBranchingGenerator generator;
    manifold = generator.generate(config);

    // STEP 2 TEST: AudioEngine/SplatAudioProcessor registration re-enabled.
    // One passthrough-only SplatAudioProcessor per splat (no effects, no
    // parameter slots yet - just the built-in occlusion-gain multiply).
    for (size_t i = 0; i < manifold.size(); ++i)
    {
        auto splatProcessor = std::make_unique<ags::engine::SplatAudioProcessor>();

        const auto delayIndex = splatProcessor->addEffect(
            std::make_unique<ags::engine::SpiralDelayProcessor>());

        // paramId 0: delay time in samples. Fixed default for now.
        splatProcessor->addParameterSlot(
            delayIndex,
            0,
            std::make_unique<ags::params::EffectParameter>(1.0f, 44100.0f, 8000.0f),
            ags::params::GMMBinding{ ags::params::GMMAttribute::None, false });

        // paramId 1: feedback. Fixed default for now.
        splatProcessor->addParameterSlot(
            delayIndex,
            1,
            std::make_unique<ags::params::EffectParameter>(0.0f, 0.95f, 0.3f),
            ags::params::GMMBinding{ ags::params::GMMAttribute::None, false });

        // paramId 2: wet mix, GMM-bound to Density - this is the one
        // parameter actually driven by manifold geometry for this pass.
        splatProcessor->addParameterSlot(
            delayIndex,
            2,
            std::make_unique<ags::params::EffectParameter>(0.0f, 1.0f, 0.0f),
            ags::params::GMMBinding{ ags::params::GMMAttribute::Density, false });

        audioEngine.addSplatProcessor(std::move(splatProcessor));
    }

    addParameter(rotationYaw = new juce::AudioParameterFloat(
        "rotationYaw", "Manifold Yaw",
        -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, 0.0f));

    addParameter(rotationPitch = new juce::AudioParameterFloat(
        "rotationPitch", "Manifold Pitch",
        -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, 0.0f));
}

void AgsAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // STEP 2 TEST: AudioEngine setup re-enabled.
    audioEngine.setSampleRate(static_cast<float>(sampleRate));
    audioEngine.reset();

    perSplatBuffer.setSize(static_cast<int>(manifold.size()), samplesPerBlock);
}

void AgsAudioProcessor::releaseResources() {}

void AgsAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    // STEP 2 TEST: AudioEngine now actually processes audio. This is the
    // first point where the ~1,280-splat per-sample loop runs under a
    // live host, since Step 1 never touched AudioEngine at all.
    ags::manifold::RotationAngles angles;
    angles.yawRadians = rotationYaw->get();
    angles.pitchRadians = rotationPitch->get();
    audioEngine.setRotation(angles);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels == 0 || manifold.size() == 0)
        return;
    
    const auto* inputChannel = buffer.getReadPointer(0);

    audioEngine.processBlock(inputChannel, numSamples, manifold, perSplatBuffer);

    const size_t numSplatChannels = manifold.size();
    const float scale = numSplatChannels > 0
        ? 1.0f / static_cast<float>(numSplatChannels)
        : 1.0f;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* out = buffer.getWritePointer(ch);
        std::fill(out, out + numSamples, 0.0f);

        for (size_t splatIdx = 0; splatIdx < numSplatChannels; ++splatIdx)
        {
            const auto* splatChannel = perSplatBuffer.getReadPointer(static_cast<int>(splatIdx));
            for (int i = 0; i < numSamples; ++i)
                out[i] += splatChannel[i] * scale;
        }
    }
}

juce::AudioProcessorEditor* AgsAudioProcessor::createEditor()
{
    return new AgsAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AgsAudioProcessor();
}
