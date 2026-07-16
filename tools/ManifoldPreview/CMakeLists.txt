juce_add_gui_app(ManifoldPreview
        PRODUCT_NAME "Manifold Preview"
)

target_sources(ManifoldPreview
        PRIVATE
        Main.cpp
)

target_link_libraries(ManifoldPreview
        PRIVATE
        ags-manifold
        juce::juce_gui_extra
        juce::juce_opengl
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_features(ManifoldPreview PRIVATE cxx_std_17)

target_compile_definitions(ManifoldPreview
        PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

juce_generate_juce_header(ManifoldPreview)