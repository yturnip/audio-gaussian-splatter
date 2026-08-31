add_library(ags-engine INTERFACE
        include/ags/engine/EffectProcessor.h
        include/ags/engine/SpiralDelayProcessor.h
        include/ags/engine/EffectChain.h
        include/ags/engine/SplatAudioProcessor.h
        include/ags/engine/AudioEngine.h
        include/ags/engine/SpiralTremoloProcessor.h
        include/ags/engine/EffectRegistry.h
        include/ags/engine/SpiralWahWahProcessor.h
        include/ags/engine/SpiralRingModProcessor.h
        include/ags/engine/SpiralFlangerProcessor.h
        include/ags/engine/SpiralVibratoProcessor.h
        include/ags/engine/SpiralDistortionProcessor.h
        include/ags/engine/SpiralCompressorProcessor.h
        include/ags/engine/SpiralExpanderGateProcessor.h
        include/ags/engine/SpiralPhaseVocoderProcessor.h)

target_include_directories(ags-engine INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ags-engine INTERFACE
        spiral-dsp
        ags-params
)