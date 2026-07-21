add_library(ags-params STATIC
        include/ags/params/EffectParameter.h
        src/EffectParameter.cpp
        include/ags/params/GMMBinding.h
        include/ags/params/ParameterMapper.h
        src/ParameterMapper.cpp
        include/ags/params/SplatOcclusion.h
)

target_include_directories(ags-params
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ags-params
        PUBLIC
        juce::juce_core
        ags-manifold
)