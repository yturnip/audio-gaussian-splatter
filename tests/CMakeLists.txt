add_executable(ags-manifold-tests
        src/TestRunner.cpp
        src/GeneratorTests.cpp
        src/ManifoldRotatorTests.cpp
)

target_link_libraries(ags-manifold-tests
    PRIVATE
        ags-manifold
        juce::juce_core
)

target_compile_features(ags-manifold-tests PRIVATE cxx_std_17)

add_test(NAME ags-manifold-tests COMMAND ags-manifold-tests)