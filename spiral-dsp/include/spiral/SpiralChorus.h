/*
    SpiralLab DSP Library
    https://digitalmedia.ok.ubc.ca/spiral/
 
    Implementation by Yohanes Turnip,
    inspired by flanger examples in Reiss & McPherson,
    "Audio Effects: Theory, Implementation and Application", 2nd ed.
*/

#pragma once
#include <vector>
#include <cmath>

class SpiralChorus
{
public:
    void prepare (double sampleRate, int maxDelaySamples, int numVoicesToUse)
    {
        sr = sampleRate;
        
        if (numVoicesToUse < 1)
            numVoicesToUse = 1;
        
        numVoices = numVoicesToUse;
        
        buffers.clear();
        writePositions.clear();
        lfoPhases.clear();
        
        buffers.resize (numVoices);
        writePositions.resize (numVoices);
        lfoPhases.resize (numVoices);
        
        for (int v = 0; v < numVoices; ++v)
        {
            buffers[v].assign (maxDelaySamples, 0.0f);
            writePositions[v] = 0;
            
            lfoPhases[v] = (float) v / (float) numVoices;
        }
    }
    
    void reset()
    {
        for (int v = 0; v < numVoices; ++v)
        {
            std::fill (buffers[v].begin(), buffers[v].end(), 0.0f);
            writePositions[v] = 0;
            lfoPhases[v] = (float) v / (float) numVoices;
        }
    }
    
    void setFrequency (float freqHz)        { frequency = freqHz; }
    void setBaseDelaySamples (float bsd)    { baseDelay = bsd; }
    void setSweepSamples (float sweep)      { sweepWidth = sweep; }
    void setDepth (float d)                 { depth = d; }
    void setNumVoices (int n)               { numVoices = n; }
    
    float processSample (float inData)
    {
        if (numVoices == 0 || buffers.empty())
            return inData;
        
        float sumDelayed = 0.0f;
        
        for (int v = 0; v < numVoices; v++)
        {
            auto& buffer    = buffers[v];
            int& wp         = writePositions[v];
            
            int bufLen = (int) buffer.size();
            if (bufLen <= 4)
                continue;
            
            buffer[wp] = inData;
            
            float lfo = std::sinf (2.0f * float(M_PI) * lfoPhases[v]);
            
            float currentDelay = baseDelay + sweepWidth * (0.5f + 0.5f * lfo);
            
            float readIndex = (float) wp - currentDelay;
            
            while (readIndex < 0.0f)
                readIndex += (float) bufLen;
            while (readIndex >= (float) bufLen)
                readIndex -= (float) bufLen;
            
            int index0 = (int) std::floor (readIndex);
            int index1 = (index0 + 1) % bufLen;
            float frac = readIndex - float (index0);
            
            float s0 = buffer[index0];
            float s1 = buffer[index1];
            
            float delayed = (1.0f - frac) * s0 + frac * s1;
            
            sumDelayed += delayed;
            
            if (++wp >= bufLen)
                wp = 0;
        
            lfoPhases[v] += frequency / (float) sr;
            if (lfoPhases[v] >= 1.0f)
                lfoPhases[v] -= 1.0f;
        }
        float avgDelayed = sumDelayed / (float) numVoices;
        
        float outData = inData + depth * avgDelayed;
        
        return outData;
    }
    
private:
    int numVoices = 0;
    
    std::vector<std::vector<float>> buffers;
    std::vector<int> writePositions;
    std::vector<float> lfoPhases;

    double sr = 44100.0;
    float frequency = 0.5f;
    float baseDelay = 0.02f * 44100.0f;
    float sweepWidth = 0.005f * 44100.0f;
    float depth = 0.7f;
};


