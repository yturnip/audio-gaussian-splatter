add_library(ags-engine INTERFACE
        include/ags/engine/EffectProcessor.h
        include/ags/engine/SpiralDelayProcessor.h
        include/ags/engine/EffectChain.h
        include/ags/engine/SplatAudioProcessor.h
        include/ags/engine/AudioEngine.h)

target_include_directories(ags-engine INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ags-engine INTERFACE
        spiral-dsp
        ags-params
)