juce_add_plugin(ags-app
        PRODUCT_NAME            "Audio Gaussian Splatter"
        COMPANY_NAME            "SPIRAL Lab"
        BUNDLE_ID               "com.spirallab.ags"
        PLUGIN_MANUFACTURER_CODE "Sprl"
        PLUGIN_CODE             "Agsp"
        FORMATS                 Standalone
        IS_SYNTH                FALSE
        NEEDS_MIDI_INPUT        FALSE
        NEEDS_MIDI_OUTPUT       FALSE
        IS_MIDI_EFFECT          FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        VST3_CATEGORIES         "Fx"
)

target_sources(ags-app
    PRIVATE
        src/PluginProcessor.h
        src/PluginProcessor.cpp
        src/PluginEditor.h
        src/PluginEditor.cpp
)

target_compile_features(ags-app PUBLIC cxx_std_17)

target_link_libraries(ags-app
    PRIVATE
        spiral-dsp
        ags-manifold
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_opengl
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)