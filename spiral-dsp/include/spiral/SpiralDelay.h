/*
    SpiralLab DSP Library
    https://digitalmedia.ok.ubc.ca/spiral/
 
    This file is based on example code from:
    Reiss, J.D. & McPherson, A.P.
    "Audio Effects: Theory, Implementation and Application", 2nd ed., 2026.
    (Delay example from Chapter 3: Delay Line Effects)
*/

#pragma once
#include <vector>

class SpiralDelay
{
public:
    void prepare (double sampleRate, int maxDelaySamples)
    {
        sr = sampleRate;
        buffer.resize (maxDelaySamples, 0.0f);
        bufLen = maxDelaySamples;
        writePos = 0;
        readPos = 0;
        feedback = 0.5f;
        wet = 0.5f;
    }
    
    void reset()
    {
        std::fill (buffer.begin(), buffer.end(), 0.0f);
        writePos = 0;
    }
    
    void setDelaySamples (int delaySamples)
    {
        if (bufLen == 0) return;
        
        if (delaySamples < 1)
            delaySamples = 1;
        if (delaySamples >= bufLen)
            delaySamples = bufLen - 1;
        
        delay = delaySamples;
        readPos = writePos - delay;
        if (readPos < 0)
            readPos += bufLen;
    }
    
    void setFeedback (float fb) { feedback = fb; }
    void setWet (float w) { wet =  w; }
    
    float processSample (float inData)
    {
        float delayed = buffer[readPos];
        float outData = (1.0f - wet) * inData + wet * delayed;
        buffer[writePos] = inData + delayed * feedback;
        
        if (++writePos >= bufLen) writePos = 0;
        if (++readPos >= bufLen) readPos = 0;
        
        return outData;
    }
    
private:
    std::vector<float> buffer;
    int bufLen = 0;
    int writePos = 0;
    int readPos = 0;
    int delay = 1;
    
    double sr = 44100.0;
    float feedback = 0.5f;
    float wet = 0.5f;
};
