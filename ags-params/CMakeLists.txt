add_library(ags-params STATIC
        include/ags/params/EffectParameter.h
        src/EffectParameter.cpp
)

target_include_directories(ags-params
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ags-params
        PUBLIC
        juce::juce_core
)