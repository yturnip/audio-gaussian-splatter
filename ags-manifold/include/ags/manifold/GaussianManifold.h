//
// Created by Yohanes Turnip on 2026-07-02.
//

#ifndef AUDIOGAUSSIANSPLATTER_GAUSSIANMANIFOLD_H
#define AUDIOGAUSSIANSPLATTER_GAUSSIANMANIFOLD_H

#endif //AUDIOGAUSSIANSPLATTER_GAUSSIANMANIFOLD_H

#pragma once
#include <vector>
#include <cstddef>
#include "GaussianSplat.h"

namespace ags::manifold
{
    class GaussianManifold
    {
    public:
        using Container = std::vector<GaussianSplat>;
        using value_type = GaussianSplat;
        using size_type = Container::size_type;

        GaussianManifold() = default;
        explicit GaussianManifold(Container splats) : splats_(std::move(splats)) {}

        [[nodiscard]] bool empty() const noexcept { return splats_.empty(); }
        [[nodiscard]] size_type size() const noexcept { return splats_.size(); }

        [[nodiscard]] const Container& splats() const noexcept { return splats_; }
        [[nodiscard]] Container& splats() noexcept { return splats_; }

        void addSplat(const GaussianSplat& splat) { splats_.push_back(splat); }
        void addSplat(GaussianSplat&& splat) { splats_.push_back(std::move(splat)); }

    private:
        Container splats_;
    };
}
