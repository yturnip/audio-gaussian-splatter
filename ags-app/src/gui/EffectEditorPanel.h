//
// Created by Yohanes Turnip on 2026-08-16.
//

#ifndef AUDIOGAUSSIANSPLATTER_EFFECTEDITORPANEL_H
#define AUDIOGAUSSIANSPLATTER_EFFECTEDITORPANEL_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include "ags/engine/EffectProcessor.h"
#include "ags/engine/AudioEngine.h"
#include "ags/engine/SplatAudioProcessor.h"
#include "ags/params/GMMBinding.h"

namespace ags::app
{
    class EffectEditorPanel final : public juce::Component,
                                     private juce::Slider::Listener,
                                     private juce::ComboBox::Listener
    {
    public:
        // splatProcessor: representative SplatAudioProcessor instance whose
        // bindings this panel edits. Bindings are chain-level, identical across
        // every splat's copy of the same effect, so any one instance works --
        // this panel never reads/writes per-splat live values.
        // effectIndex: which effect within that chain this panel displays.
        EffectEditorPanel(ags::engine::AudioEngine& engine,
                        ags::engine::SplatAudioProcessor& representativeSplat,
                        size_t effectIndex,
                        std::vector<ags::engine::EffectParameterDescriptor> descriptors);
        ~EffectEditorPanel() override;

        void paint(juce::Graphics& g) override;
        void resized() override;

    private:
        void sliderValueChanged(juce::Slider* slider) override;
        void comboBoxChanged(juce::ComboBox* comboBox) override;

        struct ParamRow
        {
            ags::engine::EffectParameterDescriptor descriptor;
            juce::Label nameLabel;
            juce::Slider valueSlider;
            juce::ComboBox bindingCombo;
        };

        void refreshRowFromState(ParamRow& row);
        static ags::params::GMMAttribute comboIndexToAttribute(int comboBoxIndex);
        static int attributeToComboIndex(ags::params::GMMAttribute attribute);

        ags::engine::AudioEngine& audioEngine;
        ags::engine::SplatAudioProcessor& displayProcessor;
        size_t effectIdx;
        std::vector<ags::engine::EffectParameterDescriptor> paramDescriptors;
        juce::OwnedArray<ParamRow> rows;

        static constexpr int rowHeight = 28;
        static constexpr int comboWidth = 90;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectEditorPanel)
    };
}

#endif //AUDIOGAUSSIANSPLATTER_EFFECTEDITORPANEL_H