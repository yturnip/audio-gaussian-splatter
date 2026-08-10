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

        // Marks the cached rotated manifold dirty rather than rotating
        // immediately. Rotation angles change at parameter/GUI rate, not
        // per-sample, so the actual rotate() call in processSample() below
        // only re-runs when the angle actually changed (or a different
        // manifold instance is passed in) instead of on every single sample.
        void setRotation(const ags::manifold::RotationAngles& angles)
        {
            currentRotation = angles;
            rotationDirty = true;
        }

        // Runs every splat's processor over the same input sample, averaging
        // into mono output. The manifold is rotated at most once per distinct
        // (angles, manifold) pair via the cache below, not once per call.
        float processSample(float inputSample, const ags::manifold::GaussianManifold& manifold)
        {
            if (rotationDirty || cachedManifoldPtr != &manifold)
            {
                cachedRotatedManifold = rotator.rotate(manifold, currentRotation);
                cachedManifoldPtr = &manifold;
                rotationDirty = false;
            }

            const auto& rotatedSplats = cachedRotatedManifold.splats();

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

        ags::manifold::GaussianManifold cachedRotatedManifold;
        const ags::manifold::GaussianManifold* cachedManifoldPtr { nullptr };
        bool rotationDirty { true };
    };
}
#endif //AUDIOGAUSSIANSPLATTER_AUDIOENGINE_H
