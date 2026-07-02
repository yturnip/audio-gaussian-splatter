/*
    SpiralLab DSP Library
    https://digitalmedia.ok.ubc.ca/spiral/
 
    This file is based on equation from:
    Reiss, J.D. & McPherson, A.P.
    "Audio Effects: Theory, Implementation and Application", 2nd ed., 2026.
    (Schroeder's allpass equation from Chapter 10: Reverberation)
*/

#pragma once
#include <vector>

class SpiralAllpassSchroeder
{
public:
    void prepare (double sampleRate, int maxDelaySamples)
    {
        sr = sampleRate;
        
        xBuffer.assign (maxDelaySamples, 0.0f);
        yBuffer.assign (maxDelaySamples, 0.0f);
        
        bufLen = maxDelaySamples;
        writePos = 0;
        delay = 1;
        g = 0.7f;
    }
    
    void reset()
    {
        std::fill (xBuffer.begin(), xBuffer.end(), 0.0f);
        std::fill (yBuffer.begin(), yBuffer.end(), 0.0f);
        writePos = 0;
    }
    
    void setDelaySamples (int d)
    {
        if (bufLen == 0) return;
        
        if (d < 1)
            d = 1;
        if (d >= bufLen)
            d = bufLen - 1;
        
        delay = d;
    }
    
    void setGain (float newG) { g = newG; }
    
    float processSample (float x)
    {
        int readIndex = writePos - delay;
        if (readIndex < 0)
            readIndex += bufLen;
        
        float xDelayed = xBuffer[readIndex];
        float yDelayed = yBuffer[readIndex];
        
        float y = -g * x + xDelayed + g * yDelayed;
        
        xBuffer[writePos] = x;
        yBuffer[writePos] = y;
        
        if (++writePos >= bufLen)
            writePos = 0;
        
        return y;
    }
    
private:
    std::vector<float> xBuffer, yBuffer;
    int bufLen = 0;
    int writePos = 0;
    int delay = 1;
    
    double sr = 44100.0;
    float g = 0.7f;
};
