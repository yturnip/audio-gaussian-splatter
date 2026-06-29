#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class AgsAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AgsAudioProcessorEditor (AgsAudioProcessor& p)
        : AudioProcessorEditor (p)
    {
        setSize (800, 600);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::darkgrey);
        g.setColour (juce::Colours::white);
        g.setFont (20.0f);
        g.drawText ("Audio Gaussian Splatter", getLocalBounds(), juce::Justification::centred);
    }

    void resized() override {}
};