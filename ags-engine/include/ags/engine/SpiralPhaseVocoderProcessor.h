//
// Created by Yohanes Turnip on 2026-08-31.
//

#ifndef AUDIOGAUSSIANSPLATTER_SPIRALPHASEVOCODERPROCESSOR_H
#define AUDIOGAUSSIANSPLATTER_SPIRALPHASEVOCODERPROCESSOR_H

#include "ags/engine/EffectProcessor.h"
#include "spiral/SpiralPhaseVocoder.h"

namespace ags::engine
{
    namespace detail
    {
        template <typename VocoderType>
        class SpiralPhaseVocoderProcessorBase : public EffectProcessor
        {
        public:
            void setSampleRate(float newSampleRate) override
            {
                sampleRate = newSampleRate;
                vocoder.setSampleRate(newSampleRate);
                if (!prepared)
                {
                    vocoder.prepare(defaultFftSize, defaultHopSize);
                    prepared = true;
                }
            }

            void reset() override
            {
                vocoder.reset();
            }

            void setParameter(int paramId, float value) override
            {
                (void) paramId;
                (void) value;
            }

            float processSample(float inputSample) override
            {
                float sample = inputSample;
                vocoder.processBlock(&sample, 1);
                return sample;
            }

            void processBlock(float* buffer, int numSamples) override
            {
                vocoder.processBlock(buffer, static_cast<unsigned int>(numSamples));
            }

            [[nodiscard]] std::vector<EffectParameterDescriptor> getParameterDescriptors() const override
            {
                return {
                        { 0, "FFT Size (log2)", 8.0f, 12.0f, 10.0f, 1.0f },
                        { 1, "Hop Size", 64.0f, 1024.0f, 256.0f, 0.0f }
                };
            }

        protected:
            VocoderType vocoder;
            float sampleRate { 44100.0f };
            bool prepared { false };
            static constexpr unsigned int defaultFftSize = 1024;
            static constexpr unsigned int defaultHopSize = 256;
        };
    }

    class SpiralRobotVocoderProcessor final
        : public detail::SpiralPhaseVocoderProcessorBase<SpiralRobotVocoder>
    {
    public:
        [[nodiscard]] std::string getName() const override { return "Robot Vocoder"; }
    };

    class SpiralWhisperVocoderProcessor final
        : public detail::SpiralPhaseVocoderProcessorBase<SpiralWhisperVocoder>
    {
    public:
        [[nodiscard]] std::string getName() const override { return "Whisper Vocoder"; }
    };

    class SpiralPitchShifterVocoderProcessor final
        : public detail::SpiralPhaseVocoderProcessorBase<SpiralPitchShifterVocoder>
    {
    public:
        [[nodiscard]] std::string getName() const override { return "Pitch Shifter"; }

        void setParameter(int paramId, float value) override
        {
            if (paramId == 2)
            {
                vocoder.setPitchShift(value);
                return;
            }
            detail::SpiralPhaseVocoderProcessorBase<SpiralPitchShifterVocoder>::setParameter(paramId, value);
        }

        [[nodiscard]] std::vector<EffectParameterDescriptor> getParameterDescriptors() const override
        {
            auto descriptors = detail::SpiralPhaseVocoderProcessorBase<SpiralPitchShifterVocoder>
                ::getParameterDescriptors();
            descriptors.push_back({ 2, "Pitch Shift", 0.25f, 4.0f, 1.0f, 0.0f });
            return descriptors;
        }
    };
}

#endif //AUDIOGAUSSIANSPLATTER_SPIRALPHASEVOCODERPROCESSOR_H
