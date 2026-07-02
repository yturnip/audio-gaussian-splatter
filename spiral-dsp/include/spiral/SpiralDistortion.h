/*
    SpiralLab DSP Library
    https://digitalmedia.ok.ubc.ca/spiral/
 
    This file is based on example code from:
    Reiss, J.D. & McPherson, A.P.
    "Audio Effects: Theory, Implementation and Application", 2nd ed., 2026.
    (Distortion example from Chapter 6: Distortion)
*/

#pragma once
#include <vector>
#include <cmath>

class SpiralDistortion
{
public:
    enum DistortionType
    {
        HardClipping = 0,
        SoftClipping,
        SoftClippingExponential,
        FullWaveRectifier,
        HalfWaveRectifier
    };
    
    void setSampleRate (float sr)       { sampleRate = sr; }
    void setInputGainDb (float dB)
    {
        inputGainDb = dB;
        inputGainLin = std::pow (10.0f, inputGainDb / 20.0f);
    }
    void setOutputGainDb (float dB)
    {
        outputGainDb = dB;
        outputGainLin = std::pow (10.0f, outputGainDb / 20.0f);
    }
    void setDistortionType (DistortionType t)      { type = t; }
    
    void reset()    {}
    
    float processSample (float inData)
    {
        const float input = inData * inputGainLin;
        
        float output = 0.0f;
        
        switch (type)
        {
            case HardClipping:
            {
                const float threshold = 1.0f;
                if      (input > threshold) output = threshold;
                else if (input < -threshold) output = -threshold;
                else    output = input;
                break;
            }
            
            case SoftClipping:
            {
                const float threshold1 = 1.0f/3.0f;
                const float threshold2 = 2.0f/3.0f;
                
                if (input > threshold2) output = 1.0f;
                else if (input > threshold1) output = (3.0f - (2.0f + 3.0f * input) * (2.0f + 3.0f * input)) / 3.0f;
                else if (input < -threshold2) output = 1.0f;
                else if (input < -threshold1) output = -(3.0f - (2.0f + 3.0f * input) * (2.0f + 3.0f * input)) / 3.0f;
                else output = 2.0f * input;
                break;
            }
            
            case SoftClippingExponential:
            {
                if (input > 0) output = 1.0f - std::exp (-input);
                else output = -1.0f + std::exp (input);
                break;
            }
            
            case FullWaveRectifier:
            {
                output = std::fabs (input);
                break;
            }
            
            case HalfWaveRectifier:
            {
                if (input > 0) output = input;
                else output = 0;
                break;
            }
        }
        
        output *= outputGainLin;
        return output;
    }
    
private:
    float sampleRate            = 44100.0f;
    
    float inputGainDb            = 0.0f;
    float inputGainLin          = 1.0f;
    
    float outputGainDb          = 0.0f;
    float outputGainLin         = 1.0f;
    
    DistortionType type   =  HardClipping;
};
