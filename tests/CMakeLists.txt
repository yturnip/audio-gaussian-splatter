add_executable(ags-manifold-tests
        src/TestRunner.cpp
        src/GeneratorTests.cpp
        src/ManifoldRotatorTests.cpp
        src/EffectParameterTests.cpp
        src/ParameterMapperTests.cpp
        src/SplatOcclusionTests.cpp
        src/EffectChainTests.cpp
        src/SplatAudioProcessorTests.cpp
)

target_link_libraries(ags-manifold-tests
    PRIVATE
        ags-manifold
        ags-params
        ags-engine
        juce::juce_core
)

add_executable(eccentricity-distribution-test
        src/EccentricityDistributionTest.cpp)

target_link_libraries(eccentricity-distribution-test
    PRIVATE
        ags-manifold)

target_compile_features(ags-manifold-tests PRIVATE cxx_std_17)

add_test(NAME ags-manifold-tests COMMAND ags-manifold-tests)