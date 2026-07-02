/*
    SpiralLab DSP Library
    https://digitalmedia.ok.ubc.ca/spiral/
 
    Based on: Reiss & McPherson, "Audio Effects: Theory, Implementation and Application"
    2nd ed., Chapter 10: Reverberation — Overlap-Add Convolutional Reverb (Eq. 10.22–10.27)
 */

#pragma once
#include <vector>

class SpiralConvolutionReverb
{
public:
    void prepare (double sampleRate, int blockSize, const std::vector<float>& ir)
    {
        sr = sampleRate;
        N  = blockSize;

        std::vector<float> safeIR = ir.empty() ? std::vector<float>{1.0f} : ir;

        fftOrder = 1;
        while ((1 << fftOrder) < 2 * N)
            ++fftOrder;
        fftSize = 1 << fftOrder;

        fft = std::make_unique<juce::dsp::FFT> (fftOrder);

        const int irLength    = (int) safeIR.size();
        numPartitions = (irLength + N - 1) / N;

        irPartitions.clear();
        irPartitions.resize (numPartitions,
                             std::vector<juce::dsp::Complex<float>> (fftSize, {0.0f, 0.0f}));

        for (int p = 0; p < numPartitions; ++p)
        {
            int start = p * N;
            for (int i = 0; i < N && (start + i) < irLength; ++i)
            {
                irPartitions[p][i].real (safeIR[start + i]);
                irPartitions[p][i].imag (0.0f);
            }
        
            for (int i = N; i < fftSize; ++i)
            {
                irPartitions[p][i].real (0.0f);
                irPartitions[p][i].imag (0.0f);
            }
            fft->perform (irPartitions[p].data(), irPartitions[p].data(), false);
        }

        inputHistory.clear();
        inputHistory.resize (numPartitions,
                             std::vector<juce::dsp::Complex<float>> (fftSize, {0.0f, 0.0f}));
        historyIndex = 0;

        freqDomainInput.assign (fftSize, {0.0f, 0.0f});
        accumulator    .assign (fftSize, {0.0f, 0.0f});
        timeDomainOut  .assign (fftSize, 0.0f);
        tailBuffer     .assign (N,       0.0f);
    }

    void processBlock (const float* input, float* output, int numSamples)
    {
        jassert (numSamples <= N);
        jassert (fft != nullptr);

        freqDomainInput.assign (fftSize, {0.0f, 0.0f});
        for (int i = 0; i < numSamples; ++i)
            freqDomainInput[i] = {input[i], 0.0f};

        fft->perform (freqDomainInput.data(), freqDomainInput.data(), false);

        inputHistory[historyIndex] = freqDomainInput;

        accumulator.assign (fftSize, {0.0f, 0.0f});

        for (int p = 0; p < numPartitions; ++p)
        {
            int hi = (historyIndex - p + numPartitions) % numPartitions;

            for (int i = 0; i < fftSize; ++i)
            {
                auto& a = inputHistory[hi][i];
                const auto& b = irPartitions[p][i];
                accumulator[i].real (accumulator[i].real() + a.real() * b.real() - a.imag() * b.imag());
                accumulator[i].imag (accumulator[i].imag() + a.real() * b.imag() + a.imag() * b.real());
            }
        }

        fft->perform (accumulator.data(), accumulator.data(), true);

        const float norm = 1.0f / static_cast<float> (fftSize);
        for (int i = 0; i < fftSize; ++i)
            timeDomainOut[i] = accumulator[i].real() * norm;

        for (int i = 0; i < numSamples; ++i)
            output[i] = timeDomainOut[i] + tailBuffer[i];

        for (int i = 0; i < N; ++i)
            tailBuffer[i] = timeDomainOut[i + N];

        historyIndex = (historyIndex + 1) % numPartitions;
    }

    void reset()
    {
        for (auto& h : inputHistory) std::fill (h.begin(), h.end(), juce::dsp::Complex<float>{0.f, 0.f});
        std::fill (tailBuffer.begin(), tailBuffer.end(), 0.0f);
        historyIndex = 0;
    }

private:
    double sr          = 44100.0;
    int    N           = 0;
    int    fftOrder    = 0;
    int    fftSize     = 0;
    int    numPartitions = 0;
    int    historyIndex  = 0;

    std::unique_ptr<juce::dsp::FFT> fft;

    std::vector<std::vector<juce::dsp::Complex<float>>> irPartitions;

    std::vector<std::vector<juce::dsp::Complex<float>>> inputHistory;

    std::vector<juce::dsp::Complex<float>> freqDomainInput;
    std::vector<juce::dsp::Complex<float>> accumulator;
    std::vector<float>                     timeDomainOut;
    std::vector<float>                     tailBuffer;
};
