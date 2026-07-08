add_library(ags-manifold STATIC
        include/ags/manifold/GaussianSplat.h
        include/ags/manifold/GaussianManifold.h
        include/ags/manifold/Generators/GeneratorConfig.h
        include/ags/manifold/Generators/SphereBranchingGenerator.h
        include/ags/manifold/Generators/DomeBranchingGenerator.h
        src/GaussianManifold.cpp
        src/Generators/SphereBranchingGenerator.cpp
        src/Generators/DomeBranchingGenerator.cpp
)

target_include_directories(ags-manifold
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ags-manifold
        PUBLIC
        glm::glm
        juce::juce_core
)
