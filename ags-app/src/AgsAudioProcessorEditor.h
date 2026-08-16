#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
// #include <juce_opengl/juce_opengl.h>
#include "AgsAudioProcessor.h"
// STEP 1 TEST: GUI/OpenGL dependencies disabled to isolate whether the
// audio path works before any GUI/renderer code is involved.
// #include "gui/ManifoldRenderer.h"
// #include "gui/ManifoldViewport.h"
// #include "gui/RotationPad.h"

class AgsAudioProcessorEditor final : public juce::AudioProcessorEditor
                                       //private ags::app::RotationPad::Listener
{
public:
    explicit AgsAudioProcessorEditor(AgsAudioProcessor& p);
    ~AgsAudioProcessorEditor() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override {}

private:
    // void rotationPadChanged(float normalizedX, float normalizedY) override;
    // void updateRenderedManifold();
    //
    // AgsAudioProcessor& processorRef;
    //
    // ags::app::ManifoldViewport manifoldViewport;
    // juce::OpenGLContext openGLContext;
    // ags::app::ManifoldRenderer manifoldRenderer;
    //
    // ags::app::RotationPad rotationPad;
    AgsAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AgsAudioProcessorEditor)
};