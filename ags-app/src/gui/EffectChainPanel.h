//
// Created by Yohanes Turnip on 2026-08-20.
//

#ifndef AUDIOGAUSSIANSPLATTER_EFFECTCHAINPANEL_H
#define AUDIOGAUSSIANSPLATTER_EFFECTCHAINPANEL_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include <vector>
#include "ags/engine/AudioEngine.h"
#include "ags/engine/SplatAudioProcessor.h"
#include "ags/engine/EffectRegistry.h"
#include "EffectEditorPanel.h"

namespace ags::app
{
    class EffectChainPanel final : public juce::Component,
                                private juce::Button::Listener
    {
    public:
        EffectChainPanel(ags::engine::AudioEngine& engine,
                      ags::engine::SplatAudioProcessor& representativeSplat);
        ~EffectChainPanel() override;

        void paint(juce::Graphics& g) override;
        void resized() override;

    private:
        void buttonClicked(juce::Button* button) override;

        void rebuildEffectList();
        void showEditorFor(size_t effectIndex);
        void showAddEffectMenu();

        struct EffectRow
        {
            size_t effectIndex { 0 };
            std::unique_ptr<juce::TextButton> nameButton;
            std::unique_ptr<juce::TextButton> bypassButton;
        };

        ags::engine::AudioEngine& audioEngine;
        ags::engine::SplatAudioProcessor& displayProcessor;

        juce::TextButton addButton { "+" };
        juce::TextButton removeButton { "-" };

        juce::OwnedArray<EffectRow> rows;
        size_t selectedEffectIndex { 0 };
        bool hasSelection { false };

        std::unique_ptr<EffectEditorPanel> editorPanel;

        static constexpr int rowHeight = 32;
        static constexpr int toolbarHeight = 36;
        static constexpr int listWidth = 140;
        static constexpr int bypassButtonWidth = 28;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectChainPanel)
    };
}

#endif //AUDIOGAUSSIANSPLATTER_EFFECTCHAINPANEL_H
