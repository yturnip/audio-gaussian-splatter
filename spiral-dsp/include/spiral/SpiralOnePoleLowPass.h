/*
    SpiralLab DSP Library
    https://digitalmedia.ok.ubc.ca/spiral/

    Implementation by Yohanes Turnip,
    based on "Audio Effects: Theory, Implementation and Application",
    2nd ed., by Reiss & McPherson, and on the
    "Audio EQ Cookbook" biquad formulas by Robert Bristow-Johnson
    (https://www.w3.org/TR/audio-eq-cookbook/).
*/

#pragma once
#include <JuceHeader.h>

class SpiralOnePoleLowPass
{
public:
    void setSampleRate (double sr)      { sampleRate = sr; }
    void setCutoff     (float fc)
    {
        cutoff = fc;
        updateCoefficients();
    }

    void reset (int numChannels)
    {
        prevY.assign ((size_t) numChannels, 0.0f);
    }

    float processSample (float x, int channel)
    {
        auto& yPrev = prevY[(size_t) channel];
        float y     = alpha * yPrev + (1.0f - alpha) * x;
        yPrev       = y;
        return y;
    }

private:
    void updateCoefficients()
    {
        const float twoPi = juce::MathConstants<float>::twoPi;
        alpha = std::exp (-twoPi * cutoff / (float) sampleRate);
    }

    double sampleRate { 44100.0 };
    float  cutoff     { 1000.0f };
    float  alpha      { 0.0f };
    std::vector<float>  prevY;
};
