#include "PluginProcessor.h"
#include "PluginEditor.h"

AgsAudioProcessor::AgsAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    ags::manifold::GeneratorConfig config;
    ags::manifold::SphereBranchingGenerator generator;
    manifold = generator.generate(config);

    addParameter(rotationYaw = new juce::AudioParameterFloat(
        "rotationYaw", "Manifold Yaw",
        -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, 0.0f));

    addParameter(rotationPitch = new juce::AudioParameterFloat(
        "rotationPitch", "Manifold Pitch",
        -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, 0.0f));
}

void AgsAudioProcessor::prepareToPlay(double, int) {}
void AgsAudioProcessor::releaseResources() {}
void AgsAudioProcessor::processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) {}

juce::AudioProcessorEditor* AgsAudioProcessor::createEditor()
{
    return new AgsAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AgsAudioProcessor();
}