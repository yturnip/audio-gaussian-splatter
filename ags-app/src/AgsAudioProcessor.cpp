#include "AgsAudioProcessor.h"
#include "AgsAudioProcessorEditor.h"

AgsAudioProcessor::AgsAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    ags::manifold::GeneratorConfig config;
    ags::manifold::SphereBranchingGenerator generator;
    manifold = generator.generate(config);

    // One SplatAudioProcessor per splat, added in manifold order so
    // AudioEngine's positional splat<->processor correspondence holds.
    for (size_t i = 0; i < manifold.size(); ++i)
        audioEngine.addSplatProcessor(std::make_unique<ags::engine::SplatAudioProcessor>());

    addParameter(rotationYaw = new juce::AudioParameterFloat(
        "rotationYaw", "Manifold Yaw",
        -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, 0.0f));

    addParameter(rotationPitch = new juce::AudioParameterFloat(
        "rotationPitch", "Manifold Pitch",
        -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, 0.0f));
}

void AgsAudioProcessor::prepareToPlay(double sampleRate, int)
{
    audioEngine.setSampleRate(static_cast<float>(sampleRate));
    audioEngine.reset();
}

void AgsAudioProcessor::releaseResources() {}

void AgsAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    // Rotation angles only change at parameter/GUI rate, not per-sample.
    // setRotation() marks AudioEngine's cached rotated manifold dirty;
    // it is only actually recomputed once, on the first processSample()
    // call below, not once per sample.
    ags::manifold::RotationAngles angles;
    angles.yawRadians = rotationYaw->get();
    angles.pitchRadians = rotationPitch->get();
    audioEngine.setRotation(angles);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels == 0)
        return;

    auto* firstChannel = buffer.getWritePointer(0);

    for (int i = 0; i < numSamples; ++i)
    {
        const float out = audioEngine.processSample(firstChannel[i], manifold);

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.getWritePointer(ch)[i] = out;
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
