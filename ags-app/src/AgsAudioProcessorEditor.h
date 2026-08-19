#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_opengl/juce_opengl.h>
#include "AgsAudioProcessor.h"
#include "gui/ManifoldRenderer.h"
#include "gui/ManifoldViewport.h"
#include "gui/RotationPad.h"
#include "gui/EffectEditorPanel.h"

class AgsAudioProcessorEditor final :
    public juce::AudioProcessorEditor,
    private ags::app::RotationPad::Listener
{
public:
    explicit AgsAudioProcessorEditor(AgsAudioProcessor& p);
    ~AgsAudioProcessorEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void rotationPadChanged(float normalizedX, float normalizedY) override;
    void updateRenderedManifold();

    AgsAudioProcessor& processorRef;

    ags::app::ManifoldViewport manifoldViewport;
    juce::OpenGLContext openGLContext;
    ags::app::ManifoldRenderer manifoldRenderer;

    ags::app::RotationPad rotationPad;

    ags::app::EffectEditorPanel tremoloPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AgsAudioProcessorEditor)
};