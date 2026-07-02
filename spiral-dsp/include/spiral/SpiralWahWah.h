/*
    SpiralLab DSP Library
    https://digitalmedia.ok.ubc.ca/spiral/
 
    Implementation by Yohanes Turnip,
    inspired by examples in Reiss & McPherson,
    "Audio Effects: Theory, Implementation and Application", 2nd ed.
*/

#pragma once
#include <vector>
#include <cmath>

class SpiralWahWah
{
public:
    void setSampleRate (double sr)
    {
        sampleRate = sr;
        filter.setSampleRate(sr);
    }
    
    void reset (int numChannels)
    {
        filter.reset(numChannels);
        lfoPhase = 0.0f;
    }
    
    void setCenterRange (float minHz, float maxHz)
    {
        minFreq = minHz;
        maxFreq = maxHz;
    }
    
    void setQ (float newQ)      { Q = newQ; }
    void setGainDb (float g)    { gainDb = g; }
    
    // Manual pedal position: 0..1 mapped into [minFreq, maxFreq]
    void setManualPosition (float pos)
    {
        manualPos = juce::jlimit(0.0f, 1.0f, pos);
        mode = Mode::Manual;
    }
    
    // LFO-based auto-wah
    void setLfoRate (float rateHz)
    {
        lfoRate = rateHz;
        mode = Mode::LFO;
    }
    
    void setDepth (float d)
    {
        depth = juce::jlimit(0.0f, 1.0f, d);
    }
    
    float processSample (float x, int channel)
    {
        float fc = computeCenterFrequency();
        
        filter.setPeakingOrNotch(fc, Q, gainDb);
        
        float y = filter.processSample(x, channel);
        
        float outData = (1.0f - depth) * x + depth * y;
        
        return outData;
    }
    
private:
    enum class Mode { Manual, LFO };
    Mode mode   { Mode::Manual };
    
    float computeCenterFrequency()
    {
        float t = 0.0f;
        
        if (mode == Mode::Manual)
        {
            t = manualPos;
        }
        else
        {
            float lfo = 0.5f + 0.5f * std::sin (2.0f * juce::MathConstants<float>::pi * lfoPhase);
            t = lfo;
            
            float phaseInc = lfoRate / (float) sampleRate;
            lfoPhase += phaseInc;
            if (lfoPhase >= 1.0f)
                lfoPhase -= 1.0f;
        }
        
        float minLog = std::log10(minFreq);
        float maxLog = std::log10(maxFreq);
        float fLog = minLog + t * (maxLog - minLog);
        return std::pow(10.0f, fLog);
    }
    
    SpiralBiquad filter;
    
    double sampleRate { 44100.0 };
    
    float minFreq       { 400.0f };
    float maxFreq       { 1200.0f };
    float Q             { 4.0f };
    float gainDb        { 8.0f };
    float depth         { 1.0f };
    
    float manualPos     { 0.0f };
    float lfoRate       { 1.0f };
    float lfoPhase      { 0.0f };
};
