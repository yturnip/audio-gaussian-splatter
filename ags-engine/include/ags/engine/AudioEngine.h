//
// Created by Yohanes Turnip on 2026-08-04.
//

#ifndef AUDIOGAUSSIANSPLATTER_AUDIOENGINE_H
#define AUDIOGAUSSIANSPLATTER_AUDIOENGINE_H
#include <vector>
#include <memory>
#include <algorithm>
#include "ags/engine/SplatAudioProcessor.h"
#include "ags/manifold/GaussianManifold.h"
#include "ags/manifold/ManifoldRotator.h"

namespace ags::engine
{
    class AudioEngine
    {
    public:
        void setSampleRate(float sampleRate)
        {
            currentSampleRate = sampleRate;
            for (auto& processor : splatProcessors)
                processor->setSampleRate(sampleRate);
        }

        void reset()
        {
            for (auto& processor : splatProcessors)
                processor->reset();
        }

        // Registers one splat's processor. The order processors are added
        // in must match the order splats appear in the manifold passed to
        // processSample(), since correspondence is purely positional. (needs some more information)
        size_t addSplatProcessor(std::unique_ptr<SplatAudioProcessor> processor)
        {
            if (currentSampleRate > 0.0f)
                processor->setSampleRate(currentSampleRate);

            splatProcessors.push_back(std::move(processor));
            return splatProcessors.size() - 1;
        }

        [[nodiscard]] size_t size() const
        {
            return splatProcessors.size();
        }

        void setRotation(const ags::manifold::RotationAngles& angles)
        {
            currentRotation = angles;
        }

        // Rotates the manifold once per block, then runs every splat's
        // processor over the same input sample, averaging into mono output.
        float processSample(float inputSample, const ags::manifold::GaussianManifold& manifold)
        {
            const auto rotatedManifold = rotator.rotate(manifold, currentRotation);
            const auto& rotatedSplats = rotatedManifold.splats();

            jassert(splatProcessors.size() == rotatedSplats.size());
            const size_t count = std::min(splatProcessors.size(), rotatedSplats.size());

            float sum = 0.0f;
            for (size_t i = 0; i < count; ++i)
            {
                splatProcessors[i]->updateParametersForBlock(rotatedSplats[i]);
                sum += splatProcessors[i]->processSample(inputSample, rotatedSplats[i]);
            }

            return count > 0 ? sum / static_cast<float>(count) : inputSample;
        }

    private:
        std::vector<std::unique_ptr<SplatAudioProcessor>> splatProcessors;
        ags::manifold::ManifoldRotator rotator;
        ags::manifold::RotationAngles currentRotation;
        float currentSampleRate { 0.0f };
    };
}
#endif //AUDIOGAUSSIANSPLATTER_AUDIOENGINE_H
