/*
    SpiralLab DSP Library
    https://digitalmedia.ok.ubc.ca/spiral/
 
    This file is based on example code from:
    Reiss, J.D. & McPherson, A.P.
    "Audio Effects: Theory, Implementation and Application", 2nd ed., 2026.
    (Vibrato example from Chapter 3: Delay Line Effects)
*/

#pragma once
#include <vector>
#include <cmath>

class SpiralVibrato
{
public:
    void prepare (double sampleRate, int maxDelaySamples)
    {
        sr = sampleRate;
        buffer.resize (maxDelaySamples, 0.0f);
        bufLen = maxDelaySamples;
        writePos = 0;
        lfoPhase = 0.0f;
    }
    
    void reset()
    {
        std::fill (buffer.begin(), buffer.end(), 0.0f);
        writePos = 0;
        lfoPhase = 0.0f;
    }
    
    void setFrequency (float freqHz)    { frequency = freqHz; }
    void setDepthSamples (float depth)  { depthSamples = depth; }
    
    float processSample (float inData)
    {
        if (bufLen <= 4)
            return inData;
        
        buffer[writePos] = inData;
        
        float lfo = std::sinf (2.0f * float(M_PI) * lfoPhase);
        
        float currentDelay = depthSamples * (0.5f + 0.5f * lfo);
        
        float readIndex = (float) writePos - currentDelay;
        
        while (readIndex < 0.0f)
            readIndex += (float) bufLen;
        while (readIndex >= (float) bufLen)
            readIndex -= (float) bufLen;
        
        int index0 = (int) std::floor (readIndex);
        int index1 = (index0 + 1) % bufLen;
        float frac = readIndex - float (index0);
        
        float s0 = buffer[index0];
        float s1 = buffer[index1];
        
        float outData = (1.0f - frac) * s0 + frac * s1;
        
        if (++writePos >= bufLen)
            writePos = 0;
        
        lfoPhase += frequency / (float) sr;
        if (lfoPhase >= 1.0f)
            lfoPhase -= 1.0f;
        
        return outData;
    }
    
private:
    std::vector<float> buffer;
    int bufLen = 0;
    int writePos = 0;
    
    double sr = 44100.0;
    float lfoPhase = 0.0f;
    float frequency = 5.0f;
    float depthSamples = 10.0f;
};
