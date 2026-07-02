/*
    SpiralLab DSP Library
    https://digitalmedia.ok.ubc.ca/spiral/
 
    This file is based on example code from:
    Reiss, J.D. & McPherson, A.P.
    "Audio Effects: Theory, Implementation and Application", 2nd ed., 2026.
    (Noise Gate & Expander example from Chapter 7: Dynamic Processing)
*/

#pragma once
#include <vector>
#include <cmath>

class SpiralExpanderGate
{
public:
    void setSampleRate (float sr)
    {
        sampleRate = sr;
        updateCoefficients();
    }
    
    void setThreshold (float dB)                { threshold_dB = dB; }
    void setRatio (float r)                     { ratio = r; }
    
    void setAttack (float ms)
    {
        attackTimeMs = ms;
        updateCoefficients();
    }
    void setRelease (float ms)
    {
        releaseTimeMs = ms;
        updateCoefficients();
    }
    
    void setFloorGain (float dB)                { floorGain_dB = dB; }
    void setMakeUpGain (float dB)               { makeUpGain_dB = dB; }
    
    void reset()
    {
        yL_prev_dB = 0.0f;
    }
    
    float computeGain (float input)
    {
        float x_g_dB;
        const float absIn = std::fabs(input);
        
        
        if (absIn < 1.0e-6f)
            x_g_dB = -120.0f;
        else
            x_g_dB = 20*log10(absIn);
        
        float y_g_dB;
        if (x_g_dB >= threshold_dB)
            y_g_dB = x_g_dB;
        else
        {
            y_g_dB = threshold_dB + (x_g_dB - threshold_dB) * ratio;
            
            const float minLevel = threshold_dB + floorGain_dB;
            if (y_g_dB < minLevel)
                y_g_dB = minLevel;
        }
        
        const float x_l_dB = x_g_dB - y_g_dB;
        
        float y_l_dB;
        if (x_l_dB > yL_prev_dB)
            y_l_dB = alphaAttack * yL_prev_dB + (1.0f - alphaAttack) * x_l_dB;
        else
            y_l_dB = alphaRelease * yL_prev_dB + (1.0f - alphaRelease) * x_l_dB;
        
        yL_prev_dB = y_l_dB;
        
        const float g_dB = makeUpGain_dB - y_l_dB;
        const float g = powf(10.0f, g_dB / 20.0f);
        
        return g;
    }
    
private:
    void updateCoefficients()
    {
        const float attackSec  = 0.001f * attackTimeMs;
        const float releaseSec = 0.001f * releaseTimeMs;
        
        if (sampleRate > 0.0f)
        {
            alphaAttack  = std::exp (-1.0f / (sampleRate * attackSec));
            alphaRelease = std::exp (-1.0f / (sampleRate * releaseSec));
        }
    }
    float sampleRate        = 44100.0f;
    
    float attackTimeMs      = 10.0f;
    float releaseTimeMs     = 100.0f;
    float alphaAttack       = 0.0f;
    float alphaRelease      = 0.0f;
    
    float threshold_dB      = 0.0f;
    float ratio             = 1.0f;
    float floorGain_dB      = -80.0f;
    float makeUpGain_dB     = 0.0f;
    
    float yL_prev_dB        = 0.0f;
};
