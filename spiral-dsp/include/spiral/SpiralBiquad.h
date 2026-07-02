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
#include <vector>
#include <JuceHeader.h>

class SpiralBiquad
{
public:
    void setSampleRate (double sr)      { sampleRate = sr; }
    
    void reset (int numChannels)
    {
        x1.assign ((size_t) numChannels, 0.0f);
        x2.assign ((size_t) numChannels, 0.0f);
        y1.assign ((size_t) numChannels, 0.0f);
        y2.assign ((size_t) numChannels, 0.0f);
    }
    
    void setCoefficients (float newB0, float newB1, float newB2, float newA1, float newA2)
    {
        b0 = newB0;
        b1 = newB1;
        b2 = newB2;
        a1 = newA1;
        a2 = newA2;
    }
    
    void setLowpass (float cutoffHz, float Q)
    {
        jassert (sampleRate > 0.0);
        auto w0 = 2.0f * juce::MathConstants<float>::pi * (cutoffHz / (float) sampleRate);
        auto cosW0 = std::cos (w0);
        auto sinW0 = std::sin (w0);
        
        auto alpha = sinW0 / (2.0f * Q);
        
        float b0_ = (1.0f - cosW0) * 0.5f;
        float b1_ = 1.0f - cosW0;
        float b2_ = (1.0f - cosW0) * 0.5f;
        float a0_ = 1.0f + alpha;
        float a1_ = -2.0f * cosW0;
        float a2_ = 1.0f - alpha;
        
        b0 = b0_ / a0_;
        b1 = b1_ / a0_;
        b2 = b2_ / a0_;
        a1 = a1_ / a0_;
        a2 = a2_ / a0_;
    }
    
    void setHighpass (float cutoffHz, float Q)
    {
        jassert (sampleRate > 0.0);
        auto w0    = 2.0f * juce::MathConstants<float>::pi * (cutoffHz / (float) sampleRate);
        auto cosW0 = std::cos (w0);
        auto sinW0 = std::sin (w0);

        auto alpha = sinW0 / (2.0f * Q);

        float b0_ =  (1.0f + cosW0) * 0.5f;
        float b1_ = -(1.0f + cosW0);
        float b2_ =  (1.0f + cosW0) * 0.5f;
        float a0_ =   1.0f + alpha;
        float a1_ =  -2.0f * cosW0;
        float a2_ =   1.0f - alpha;

        b0 = b0_ / a0_;
        b1 = b1_ / a0_;
        b2 = b2_ / a0_;
        a1 = a1_ / a0_;
        a2 = a2_ / a0_;
    }
    
    void setBandpass (float centerHz, float Q)
    {
        jassert (sampleRate > 0.0);
        
        auto w0 = 2.0f * juce::MathConstants<float>::pi * (centerHz / (float) sampleRate);
        auto cosW0 = std::cos(w0);
        auto sinW0 = std::sin(w0);
        
        auto alpha = sinW0 / (2.0f * Q);
        
        float b0_ = alpha;
        float b1_ = 0.0f;
        float b2_ = -alpha;
        float a0_ = 1.0f + alpha;
        float a1_ = -2.0f * cosW0;
        float a2_ = 1.0f - alpha;
        
        b0 = b0_ / a0_;
        b1 = b1_ / a0_;
        b2 = b2_ / a0_;
        a1 = a1_ / a0_;
        a2 = a2_ / a0_;
    }
    
    void setBandstop (float centerHz, float Q)
    {
        jassert(sampleRate > 0.0);
        
        auto w0     = 2.0f * juce::MathConstants<float>::pi * (centerHz / (float) sampleRate);
        auto cosW0  = std::cos(w0);
        auto sinW0  = std::sin(w0);
        
        auto alpha = sinW0 / (2.0f * Q);
        
        float b0_ = 1.0f;
        float b1_ = -2.0f * cosW0;
        float b2_ = 1.0f;
        float a0_ = 1.0f + alpha;
        float a1_ = -2.0f * cosW0;
        float a2_ = 1.0f - alpha;
        
        b0 = b0_ / a0_;
        b1 = b1_ / a0_;
        b2 = b2_ / a0_;
        a1 = a1_ / a0_;
        a2 = a2_ / a0_;
    }
    
    void setPeakingOrNotch (float centerHz, float Q, float gainDecibels)
    {
        jassert (sampleRate > 0.0);
        
        auto A      = std::pow(10.0f, gainDecibels / 40.0f);
        auto w0     = 2.0f * juce::MathConstants<float>::pi * (centerHz / (float) sampleRate);
        auto cosW0  = std::cos(w0);
        auto sinW0  = std::sin(w0);
        
        auto alpha = sinW0 / (2.0f * Q);
        
        float b0_ = 1.0f + alpha * A;
        float b1_ = -2.0f * cosW0;
        float b2_ = 1.0f - alpha * A;
        float a0_ = 1.0f + alpha / A;
        float a1_ = -2.0f * cosW0;
        float a2_ = 1.0f - alpha / A;
        
        b0 = b0_ / a0_;
        b1 = b1_ / a0_;
        b2 = b2_ / a0_;
        a1 = a1_ / a0_;
        a2 = a2_ / a0_;
    }
    
    void setLowShelf (float crossoverHz, float Q, float gainDecibels)
    {
        jassert(sampleRate > 0.0);
        
        auto A      = std::pow(10.0f, gainDecibels / 40.0f);
        auto w0     = 2.0f * juce::MathConstants<float>::pi * (crossoverHz / (float) sampleRate);
        auto cosW0  = std::cos(w0);
        auto sinW0  = std::sin(w0);
        
        auto alpha = sinW0 / (2.0f * Q);
        auto sqrtA = std::sqrt(A);
        
        float b0_ = A * ((A + 1.0f) - (A - 1.0f) * cosW0 + 2.0f * sqrtA * alpha);
        float b1_ = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosW0);
        float b2_ = A * ((A + 1.0f) - (A - 1.0f) * cosW0 - 2.0f * sqrtA * alpha);
        float a0_ = (A + 1.0f) + (A - 1.0f) * cosW0 + 2.0f * sqrtA * alpha;
        float a1_ = -2.0f * ((A - 1.0f) + (A + 1.0f) * cosW0);
        float a2_ = (A + 1.0f) + (A - 1.0f) * cosW0 - 2.0f * sqrtA * alpha;
        
        b0 = b0_ / a0_;
        b1 = b1_ / a0_;
        b2 = b2_ / a0_;
        a1 = a1_ / a0_;
        a2 = a2_ / a0_;
    }
    
    void setHighShelf (float crossoverHz, float Q, float gainDecibels)
    {
        jassert(sampleRate > 0.0);
        
        auto A      = std::pow(10.0f, gainDecibels / 40.0f);
        auto w0     = 2.0f * juce::MathConstants<float>::pi * (crossoverHz / (float) sampleRate);
        auto cosW0  = std::cos(w0);
        auto sinW0  = std::sin(w0);
        
        auto alpha = sinW0 / (2.0f * Q);
        auto sqrtA = std::sqrt(A);
        
        float b0_ = A * ((A + 1.0f) + (A - 1.0f) * cosW0 + 2.0f * sqrtA * alpha);
        float b1_ = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosW0);
        float b2_ = A * ((A + 1.0f) + (A - 1.0f) * cosW0 - 2.0f * sqrtA * alpha);
        float a0_ = (A + 1.0f) - (A - 1.0f) * cosW0 + 2.0f * sqrtA * alpha;
        float a1_ = 2.0f * ((A - 1.0f) - (A + 1.0f) * cosW0);
        float a2_ = (A + 1.0f) - (A - 1.0f) * cosW0 - 2.0f * sqrtA * alpha;
        
        b0 = b0_ / a0_;
        b1 = b1_ / a0_;
        b2 = b2_ / a0_;
        a1 = a1_ / a0_;
        a2 = a2_ / a0_;
    }

    float processSample (float x, int channel)
    {
        auto& x1c = x1[(size_t) channel];
        auto& x2c = x2[(size_t) channel];
        auto& y1c = y1[(size_t) channel];
        auto& y2c = y2[(size_t) channel];
        
        float y = b0 * x + b1 * x1c + b2 * x2c - a1 * y1c - a2 * y2c;
        
        x2c = x1c;
        x1c = x;
        y2c = y1c;
        y1c = y;
        
        return y;
    }

private:
    float b0 { 0.0f }, b1 { 0.0f }, b2 { 0.0f };
    float a1 { 0.0f }, a2 { 0.0f };

    double sampleRate { 44100.0 };
    
    std::vector<float> x1, x2;
    std::vector<float> y1, y2;
};
