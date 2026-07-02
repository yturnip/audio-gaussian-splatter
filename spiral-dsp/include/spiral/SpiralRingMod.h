/*
    SpiralLab DSP Library
    https://digitalmedia.ok.ubc.ca/spiral/
 
    This file is based on example code from:
    Reiss, J.D. & McPherson, A.P.
    "Audio Effects: Theory, Implementation and Application", 2nd ed., 2026.
    (Ring Modulation example from Chapter 6: Amplitude Modulation)
*/

#pragma once
#include <vector>
#include <cmath>

class SpiralRingMod
{
public:
    enum Waveform
    {
        Sine = 0,
        Triangle,
        Square,
        SquareSlopedEdges
    };
    
    void setSampleRate (float sr)
    {
        sampleRate = sr;
        inverseSampleRate = 1.0f / sampleRate;
    }
    void setCarrierFrequency (float freqHz)    { carrierFrequency = freqHz; }
    void setDepth (float d)                    { depth = d; }
    void setWaveform (Waveform wf)             { waveform = wf; }
    
    void reset()
    {
        carrierPhase = 0.0f;
    }
    
    float carrierWF (float carrierPhase, Waveform waveform)
    {
        switch (waveform)
        {
            case Triangle:
                if (carrierPhase < 0.25f)
                    return 4.0f * carrierPhase;
                else if (carrierPhase < 0.75f)
                    return 2.0f - 4.0f * carrierPhase;
                else
                    return -4.0f + 4.0f * carrierPhase;
                break;
            case Square:
                if (carrierPhase < 0.5f)
                    return 1.0f;
                else
                    return -1.0f;
                break;
            case SquareSlopedEdges:
                if (carrierPhase < 0.48f)
                    return 1.0f;
                else if (carrierPhase < 0.5f)
                    return 1.0f - 50.0f * (carrierPhase - 0.48f);
                else if (carrierPhase < 0.98f)
                    return -1.0f;
                else
                    return -1.0f + 50.0f * (carrierPhase - 0.98f);
                break;
            case Sine:
            default:
                return std::sinf (2.0f * float(M_PI) * carrierPhase);
                break;
        }
    }
    
    float processSample (float inData)
    {
        float carrier = (1.0f - depth) + depth * carrierWF(carrierPhase, waveform);
        float outData = inData * carrier;
        
        carrierPhase += carrierFrequency * inverseSampleRate;
        if (carrierPhase >= 1.0f)
            carrierPhase -= 1.0f;
        
        return outData;
    }
    
private:
    float sampleRate        = 44100.0f;
    float inverseSampleRate = 1.0f / sampleRate;
    float carrierPhase      = 0.0f;
    float depth             = 0.0f;
    float carrierFrequency  = 10.0f;
    Waveform waveform       = Sine;
};
