#include "PluginProcessor.h"
#include "PluginEditor.h"

AgsAudioProcessor::AgsAudioProcessor()
    : AudioProcessor (BusesProperties()
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{}

juce::AudioProcessorEditor* AgsAudioProcessor::createEditor()
{
    return new AgsAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AgsAudioProcessor();
}