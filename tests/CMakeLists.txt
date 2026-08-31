add_executable(ags-manifold-tests
        src/TestRunner.cpp
        src/GeneratorTests.cpp
        src/ManifoldRotatorTests.cpp
        src/EffectParameterTests.cpp
        src/ParameterMapperTests.cpp
        src/SplatOcclusionTests.cpp
        src/EffectChainTests.cpp
        src/SplatAudioProcessorTests.cpp
        src/AudioEngineTests.cpp
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

add_executable(audio-engine-scale-silence-test
        src/AudioEngineScaleSilenceTest.cpp
)

target_link_libraries(audio-engine-scale-silence-test
        PRIVATE
        ags-engine
        ags-manifold
        ags-params
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_graphics
)

target_compile_features(audio-engine-scale-silence-test PUBLIC cxx_std_17)

add_executable(audio-engine-concurrent-silence-test
        src/AudioEngineConcurrentSilenceTest.cpp
)

target_link_libraries(audio-engine-concurrent-silence-test
        PRIVATE
        ags-engine
        ags-manifold
        ags-params
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_graphics
        Threads::Threads
)

target_compile_features(audio-engine-concurrent-silence-test PUBLIC cxx_std_17)