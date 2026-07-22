//
// Created by Yohanes Turnip on 2026-07-22.
//
#include "ags/manifold/Generators/SphereBranchingGenerator.h"
#include <algorithm>
#include <iostream>
#include <vector>

int main()
{
    ags::manifold::GeneratorConfig config;
    config.numHubs = 500;
    config.pointsPerHub = 4000;
    config.spread = 0.1f;
    config.seed = 1234;

    ags::manifold::SphereBranchingGenerator gen;
    auto manifold = gen.generate(config);

    std::vector<float> ecc;
    ecc.reserve(manifold.splats().size());
    for (const auto& splat : manifold.splats())
        ecc.push_back(splat.eccentricity);

    std::sort(ecc.begin(), ecc.end());

    auto percentile = [&](double p)
    {
        size_t idx = static_cast<size_t>(p / 100.0 * (ecc.size() - 1));
        return ecc[idx];
    };

    std::cout << "N = " << ecc.size() << "\n";
    std::cout << "50th: " << percentile(50) << "\n";
    std::cout << "75th: " << percentile(75) << "\n";
    std::cout << "90th: " << percentile(90) << "\n";
    std::cout << "95th: " << percentile(95) << "\n";
    std::cout << "99th: " << percentile(99) << "\n";
    std::cout << "99.9th: " << percentile(99.9) << "\n";
    std::cout << "max: " << ecc.back() << "\n";

    return 0;
}