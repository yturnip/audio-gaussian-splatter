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

class SpiralPhaseVocoder
{
public:
    virtual ~SpiralPhaseVocoder() = default;
    
    void setSampleRate (float sr)       { sampleRate = sr; }
    
    void prepare (unsigned int fftSizeIn, unsigned int hopSizeIn)
    {
        fftSize = fftSizeIn;
        hopSize = hopSizeIn;
        hopCounter = 0;
        
        fft = std::make_unique<juce::dsp::FFT>(std::log2 (fftSize));
        
        inputBuffer.setSize(1, (int) fftSize);
        outputBuffer.setSize(1, (int) (fftSize * 2));
        
        inputBuffer.clear();
        outputBuffer.clear();
        
        inputWritePointer   = 0;
        outputWritePointer  = 0;
        outputReadPointer   = 0;
        
        timeDomainBuffer.malloc(fftSize);
        frequencyDomainBuffer.malloc(fftSize);
        
        fftWindow.malloc(fftSize);
        computeHannWindow();
        computeWindowScaleFactor();
        
        lastInputPhases.assign      (fftSize / 2 + 1, 0.0f);
        lastOutputPhases.assign     (fftSize / 2 + 1, 0.0f);
        analysisMagnitudes.assign   (fftSize / 2 + 1, 0.0f);
        analysisFrequencies.assign  (fftSize / 2 + 1, 0.0f);
        synthesisMagnitudes.assign  (fftSize / 2 + 1, 0.0f);
        synthesisFrequencies.assign (fftSize / 2 + 1, 0.0f);

    }
    
    void reset()
    {
        inputBuffer.clear();
        outputBuffer.clear();
        inputWritePointer   = 0;
        outputWritePointer  = 0;
        outputReadPointer   = 0;
        hopCounter          = 0;
    }
    
    void processBlock (float* audioData, unsigned int numSamples)
    {
        overlapAdd(audioData, numSamples);
    }
    
protected:
    void overlapAdd (float* audioData, unsigned int numSamples)
    {
        for (unsigned int n = 0; n < numSamples; ++n)
        {
            const float inputSample = audioData[n];
            inputBuffer.setSample (0, (int) inputWritePointer, inputSample);
            if (++inputWritePointer >= (unsigned int) inputBuffer.getNumSamples())
                inputWritePointer = 0;
            
            audioData[n] = outputBuffer.getSample(0, (int) outputReadPointer);
            outputBuffer.setSample (0, (int) outputReadPointer, 0.0f);
            if (++outputReadPointer >= (unsigned int) outputBuffer.getNumSamples())
                outputReadPointer = 0;
            
            if (++hopCounter >= hopSize)
            {
                hopCounter = 0;
                
                int inputIndex = (int) inputWritePointer;
                for (unsigned int i = 0; i < fftSize; ++i)
                {
                    const float x = fftWindow[i] * inputBuffer.getSample (0, inputIndex);
                    timeDomainBuffer[i].real(x);
                    timeDomainBuffer[i].imag(0.0f);
                    
                    if (++inputIndex >= inputBuffer.getNumSamples())
                        inputIndex = 0;
                }
                
                fft->perform (timeDomainBuffer, frequencyDomainBuffer, false);
                
                modification();
                
                fft->perform (frequencyDomainBuffer, timeDomainBuffer, true);
                
                int outputIndex = (int) outputWritePointer;
                for (unsigned int i = 0; i < fftSize; ++i)
                {
                    float y = outputBuffer.getSample(0, outputIndex);
                    y += timeDomainBuffer[i].real() * windowScaleFactor;
                    outputBuffer.setSample (0, outputIndex, y);
                    
                    if (++outputIndex >= outputBuffer.getNumSamples())
                        outputIndex = 0;
                }
                
                outputWritePointer += hopSize;
                if (outputWritePointer >= (unsigned int) outputBuffer.getNumSamples())
                    outputWritePointer -= outputBuffer.getNumSamples();
            }
        }
    }
    
    void computeHannWindow()
    {
        for (unsigned int n = 0; n < fftSize; ++n)
        {
            fftWindow[n] = 0.5f * (1.0f - std::cos (2.0f * float (juce::MathConstants<float>::pi) * n / (fftSize - 1)));
        }
    }
    
    void computeWindowScaleFactor()
    {
        double sumSquares = 0.0;
        for (unsigned int n = 0; n < fftSize; ++n)
            sumSquares += double (fftWindow[n] * fftWindow[n]);
        
        sumSquares /= double (fftSize);
        
        windowScaleFactor = 1.0f / (float (sumSquares) * (fftSize / hopSize));
    }
    
    virtual void modification() {}
    
    static float wrapPhase (float p)
    {
        while (p >  juce::MathConstants<float>::pi)  p -= 2.0f * juce::MathConstants<float>::pi;
        while (p < -juce::MathConstants<float>::pi)  p += 2.0f * juce::MathConstants<float>::pi;
        return p;
    }
    
    float sampleRate            = 44100.0f;
    
    juce::AudioBuffer<float> inputBuffer, outputBuffer;
    
    unsigned int inputWritePointer  = 0;
    unsigned int outputWritePointer = 0;
    unsigned int outputReadPointer  = 0;
    
    unsigned int hopCounter = 0;
    unsigned int fftSize    = 1024;
    unsigned int hopSize    = 256;
    
    float windowScaleFactor = 1.0f;
    
    std::unique_ptr<juce::dsp::FFT> fft;
    juce::HeapBlock<float> fftWindow;
    juce::HeapBlock<juce::dsp::Complex<float>> timeDomainBuffer;
    juce::HeapBlock<juce::dsp::Complex<float>> frequencyDomainBuffer;
    
    // Pitch-shift state
    std::vector<float> lastInputPhases;
    std::vector<float> lastOutputPhases;
    
    std::vector<float> analysisMagnitudes;
    std::vector<float> analysisFrequencies;
    std::vector<float> synthesisMagnitudes;
    std::vector<float> synthesisFrequencies;
    
};

class SpiralRobotVocoder : public SpiralPhaseVocoder
{
protected:
    void modification() override
    {
        for (unsigned int index = 0; index < fftSize; ++index)
        {
            float magnitude = std::abs(frequencyDomainBuffer[index]);
            
            frequencyDomainBuffer[index].real(magnitude);
            frequencyDomainBuffer[index].imag(0.0f);
        }
    }
};

class SpiralWhisperVocoder : public SpiralPhaseVocoder
{
protected:
    void modification() override
    {
        for (unsigned int index = 0; index < fftSize / 2 + 1; ++index)
        {
            float magnitude = std::abs(frequencyDomainBuffer[index]);
            
            float phase = 2.0f * juce::MathConstants<float>::pi * (float) rand() / (float) RAND_MAX;
            
            frequencyDomainBuffer[index].real(magnitude * std::cos(phase));
            frequencyDomainBuffer[index].imag(magnitude * std::sin(phase));
            
            if (index > 0 && index < fftSize / 2)
            {
                frequencyDomainBuffer[fftSize - index].real(magnitude * std::cos(phase));
                frequencyDomainBuffer[fftSize - index].imag(magnitude * std::sin(-phase));
            }
        }
    }
};

class SpiralPitchShifterVocoder : public SpiralPhaseVocoder
{
public:
    void setPitchShift (float newRatio) noexcept
    {
        pitchShift = newRatio;
    }

protected:
    void modification() override
    {
        for (int index = 0; index < fftSize / 2 + 1; ++index)
        {
            float amplitude = std::abs(frequencyDomainBuffer[index]);
            float phase     = std::arg(frequencyDomainBuffer[index]);
            
            float phaseDiff = phase - lastInputPhases[index];
            
            float binCentreFrequency = 2.0f * juce::MathConstants<float>::pi * (float) index / (float) fftSize;
            phaseDiff = wrapPhase(phaseDiff - binCentreFrequency * hopSize);
            
            float binDeviation = phaseDiff * (float) fftSize / (float) hopSize / (2.0f * juce::MathConstants<float>::pi);
            
            analysisFrequencies[index] = (float) index + binDeviation;
            
            analysisMagnitudes[index] = amplitude;
            
            lastInputPhases[index] = phase;
        }
        
        for (int index = 0; index < fftSize / 2 + 1; ++index)
        {
            synthesisMagnitudes[index] = synthesisFrequencies[index] = 0;
        }
        
        for (int index = 0; index < fftSize / 2 + 1; ++index)
        {
            int newBin = std::floor(index * pitchShift + 0.5f);
            
            if (newBin <= fftSize / 2)
            {
                synthesisMagnitudes[newBin] += analysisMagnitudes[index];
                synthesisFrequencies[newBin] = analysisFrequencies[index] * pitchShift;
            }
        }
        
        for (int index = 0; index < fftSize / 2 + 1; ++index)
        {
            float amplitude = synthesisMagnitudes[index];
            
            float binDeviation = synthesisFrequencies[index] - index;
            
            float phaseDiff = binDeviation * 2.0f * juce::MathConstants<float>::pi * (float) hopSize / (float) fftSize;
            
            float binCentreFrequency = 2.0f * juce::MathConstants<float>::pi * (float) index / (float) fftSize;
            phaseDiff += binCentreFrequency * hopSize;
            
            float outPhase = wrapPhase(lastOutputPhases[index] + phaseDiff);
            
            frequencyDomainBuffer[index].real(amplitude * std::cos(outPhase));
            frequencyDomainBuffer[index].imag(amplitude * std::sin(outPhase));
            
            if (index > 0 && index < fftSize / 2)
            {
                frequencyDomainBuffer[fftSize - index].real(amplitude * std::cos(outPhase));
                frequencyDomainBuffer[fftSize - index].imag(amplitude * std::sin(-outPhase));
            }
            
            lastOutputPhases[index] = outPhase;
        }
    }
    
    float pitchShift    = 1.0f;
};

class SpiralRandComb : public SpiralPhaseVocoder
{
public:
    void setWipe (float w)      { wipe = juce::jlimit (0.0f, 1.0f, w); }
    
    void trigger()
    {
        const int numBins = (int) fftSize / 2 + 1;
        binOrder.resize (numBins);
        std::iota(binOrder.begin(), binOrder.end(), 0);
        for (int i = numBins - 1; i > 0; --i)
        {
            int j = rng.nextInt (i + 1);
            std::swap (binOrder[i], binOrder[j]);
        }
    }
    
protected:
    void modification() override
    {
        const int numBins = (int) fftSize / 2 + 1;
        
        if ((int) binOrder.size() != numBins)
            trigger();
        
        const int binsToZero = (int) std::floor (wipe * (float) numBins);
        
        for (int i = 0; i < binsToZero; ++i)
        {
            int bin = binOrder[i];
            frequencyDomainBuffer[bin].real(0.0f);
            frequencyDomainBuffer[bin].imag(0.0f);
            
            if (bin > 0 && bin < (int) fftSize / 2)
            {
                frequencyDomainBuffer[fftSize - bin].real(0.0f);
                frequencyDomainBuffer[fftSize - bin].imag(0.0f);
            }
        }
    }
    
private:
    float wipe = 0.0f;
    std::vector<int> binOrder;
    juce::Random rng;
};

class SpiralRandWipe : public SpiralPhaseVocoder
{
public:
    void setWipe (float w)      { wipe = juce::jlimit(0.0f, 1.0f, w); }
    
    void processBSource (float* audioData, unsigned int numSamples)
    {
        bReverb.processBlock (audioData, numSamples);
    }
    
    void prepareB (unsigned int fftSizeIn, unsigned int hopSizeIn)
    {
        bReverb.setSampleRate (sampleRate);
        bReverb.prepare (fftSizeIn, hopSizeIn);
        bFftSize = fftSizeIn;
        bFreqBuffer.resize (fftSizeIn, {0.0f, 0.0f});
    }
    
    void trigger()
    {
        const int numBins = (int) fftSize / 2 + 1;
        binOrder.resize (numBins);
        std::iota(binOrder.begin(), binOrder.end(), 0);
        for (int i = numBins - 1; i > 0; --i)
        {
            int j = rng.nextInt (i + 1);
            std::swap (binOrder[i], binOrder[j]);
        }
    }
    
    void setBFreqBuffer (const std::vector<juce::dsp::Complex<float>>& buf)
    {
        bFreqBuffer = buf;
    }

protected:
    void modification() override
    {
        const int numBins = (int) fftSize / 2 + 1;
        
        if ((int) binOrder.size() != numBins)
            trigger();

        const int binsToCopy = (int) std::floor (wipe * (float) numBins);
        
        for (int i = 0; i < binsToCopy; ++i)
        {
            int bin = binOrder[i];

            if (bin < (int) bFreqBuffer.size())
            {
                frequencyDomainBuffer[bin] = bFreqBuffer[bin];

                if (bin > 0 && bin < (int) fftSize / 2)
                {
                    int mirror = (int) fftSize - bin;
                    frequencyDomainBuffer[mirror].real ( bFreqBuffer[bin].real());
                    frequencyDomainBuffer[mirror].imag (-bFreqBuffer[bin].imag());
                }
            }
        }
    }
    
private:
    float wipe = 0.0f;
    std::vector<int> binOrder;
    juce::Random rng;
    
    SpiralPhaseVocoder bReverb;
    unsigned int bFftSize = 1024;
    std::vector<juce::dsp::Complex<float>> bFreqBuffer;
};
