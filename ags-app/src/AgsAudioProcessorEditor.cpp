#include "AgsAudioProcessorEditor.h"

AgsAudioProcessorEditor::AgsAudioProcessorEditor(AgsAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(400, 200);

    // addAndMakeVisible(manifoldViewport);
    // addAndMakeVisible(rotationPad);
    // rotationPad.addListener(this);
    //
    // openGLContext.setOpenGLVersionRequired(juce::OpenGLContext::openGL3_2);
    // openGLContext.setContinuousRepainting(true);
    // openGLContext.setRenderer(&manifoldRenderer);
    // openGLContext.attachTo(manifoldViewport);
    //
    // const auto yawNorm = (processorRef.rotationYaw->get() + juce::MathConstants<float>::pi)
    //                       / (2.0f * juce::MathConstants<float>::pi);
    // const auto pitchNorm = (processorRef.rotationPitch->get() + juce::MathConstants<float>::pi)
    //                         / (2.0f * juce::MathConstants<float>::pi);
    // rotationPad.setNormalizedPosition(yawNorm, pitchNorm);
    //
    // updateRenderedManifold();
}

// AgsAudioProcessorEditor::~AgsAudioProcessorEditor()
// {
//     rotationPad.removeListener(this);
//     openGLContext.detach();
// }

void AgsAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xffe0e0e0));
    g.setColour(juce::Colours::black);
    g.setFont(16.0f);
    g.drawText("AGS - Step 1 passthrough test build", getLocalBounds(), juce::Justification::centred);
}

// void AgsAudioProcessorEditor::resized()
// {
//     // Mirrors the mockup's 2x2 layout: viewport top-right, rotation pad
//     // bottom-left. Effects chain (top-left) and density controls
//     // (bottom-right) are Phase 5 additions, not yet built.
//     auto bounds = getLocalBounds();
//
//     const int headerHeight = 80;
//     bounds.removeFromTop(headerHeight);
//
//     const int halfWidth = bounds.getWidth() / 2;
//     const int halfHeight = bounds.getHeight() / 2;
//
//     auto topRow = bounds.removeFromTop(halfHeight);
//     topRow.removeFromLeft(halfWidth); // reserved for future effects chain panel
//     manifoldViewport.setBounds(topRow.reduced(8));
//
//     auto bottomRow = bounds;
//     auto rotationArea = bottomRow.removeFromLeft(halfWidth);
//     rotationPad.setBounds(rotationArea.reduced(8));
//     // remaining bottomRow reserved for future density/generator controls
// }
//
// void AgsAudioProcessorEditor::rotationPadChanged(float normalizedX, float normalizedY)
// {
//     const auto yaw = juce::jmap(normalizedX, 0.0f, 1.0f,
//                                  -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi);
//     const auto pitch = juce::jmap(normalizedY, 0.0f, 1.0f,
//                                    -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi);
//
//     processorRef.rotationYaw->setValueNotifyingHost(processorRef.rotationYaw->convertTo0to1(yaw));
//     processorRef.rotationPitch->setValueNotifyingHost(processorRef.rotationPitch->convertTo0to1(pitch));
//
//     updateRenderedManifold();
// }
//
// void AgsAudioProcessorEditor::updateRenderedManifold()
// {
//     manifoldRenderer.setManifold(processorRef.getRotatedManifold());
// }
