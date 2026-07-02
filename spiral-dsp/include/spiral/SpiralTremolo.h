/*
    SpiralLab DSP Library
    https://digitalmedia.ok.ubc.ca/spiral/
 
    This file is based on example code from:
    Reiss, J.D. & McPherson, A.P.
    "Audio Effects: Theory, Implementation and Application", 2nd ed., 2026.
    (Tremolo example from Chapter 6: Amplitude Modulation)
*/

#pragma once
#include <vector>
#include <cmath>

class SpiralTremolo
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
    void setFrequency (float freqHz)    { frequency = freqHz; }
    void setDepth (float d)             { depth = d; }
    void setWaveform (Waveform wf)      { waveform = wf; }
    
    void reset()
    {
        lfoPhase = 0.0f;
    }
    
    float lfo (float lfoPhase, Waveform waveform)
    {
        switch (waveform)
        {
            case Triangle:
                if (lfoPhase < 0.25f)
                    return 0.5f + 2.0f*lfoPhase;
                else if (lfoPhase < 0.75f)
                    return 1.0f - 2.0f*(lfoPhase - 0.25f);
                else
                    return 2.0f*(lfoPhase - 0.75f);
                break;
            case Square:
                if (lfoPhase < 0.5f)
                    return 1.0f;
                else
                    return 0.0f;
                break;
            case SquareSlopedEdges:
                if (lfoPhase < 0.48f)
                    return 1.0f;
                else if (lfoPhase < 0.5f)
                    return 1.0f - 50.0f*(lfoPhase - 0.48f);
                else if (lfoPhase < 0.98f)
                    return 0.0f;
                else
                    return 50.0f*(lfoPhase - 0.98f);
                break;
            case Sine:
            default:
                return 0.5f + 0.5f*std::sinf (2.0f * float(M_PI) * lfoPhase);
                break;
        }
    }
    
    float processSample (float inData)
    {
        float outData = inData * (1.0f - depth*lfo(lfoPhase, waveform));
        
        lfoPhase += frequency * inverseSampleRate;
        if (lfoPhase >= 1.0f)
            lfoPhase -= 1.0f;
        
        return outData;
    }
    
private:
    float sampleRate        = 44100.0f;
    float inverseSampleRate = 1.0f / sampleRate;
    float lfoPhase          = 0.0f;
    float depth             = 0.0f;
    float frequency         = 5.0f;
    Waveform waveform       = Sine;
};
