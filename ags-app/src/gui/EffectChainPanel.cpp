//
// Created by Yohanes Turnip on 2026-08-20.
//
#include "EffectChainPanel.h"

namespace ags::app
{

EffectChainPanel::EffectChainPanel(ags::engine::AudioEngine& engine,
                                    ags::engine::SplatAudioProcessor& representativeSplat)
    : audioEngine(engine),
      displayProcessor(representativeSplat)
{
    addAndMakeVisible(addButton);
    addAndMakeVisible(removeButton);
    addButton.addListener(this);
    removeButton.addListener(this);

    rebuildEffectList();
}

EffectChainPanel::~EffectChainPanel()
{
    addButton.removeListener(this);
    removeButton.removeListener(this);

    for (auto* row : rows)
    {
        row->nameButton->removeListener(this);
        row->bypassButton->removeListener(this);
    }
}

void EffectChainPanel::rebuildEffectList()
{
    for (auto* row : rows)
    {
        row->nameButton->removeListener(this);
        row->bypassButton->removeListener(this);
    }
    rows.clear();

    const size_t count = displayProcessor.getEffectCount();

    for (size_t i = 0; i < count; ++i)
    {
        auto* row = new EffectRow();
        row->effectIndex = i;

        row->nameButton = std::make_unique<juce::TextButton>(displayProcessor.getEffectName(i));
        row->nameButton->setClickingTogglesState(false);
        row->nameButton->setColour(juce::TextButton::buttonColourId,
                                    i == selectedEffectIndex && hasSelection
                                        ? juce::Colours::lightgrey
                                        : juce::Colours::white);
        row->nameButton->setColour(juce::TextButton::textColourOffId, juce::Colours::black);
        row->nameButton->setColour(juce::TextButton::textColourOnId, juce::Colours::black);
        row->nameButton->addListener(this);
        addAndMakeVisible(*row->nameButton);

        row->bypassButton = std::make_unique<juce::TextButton>("X");
        row->bypassButton->setColour(juce::TextButton::buttonColourId,
                                      displayProcessor.isEffectBypassed(i)
                                          ? juce::Colours::darkgrey
                                          : juce::Colours::white);
        row->bypassButton->setColour(juce::TextButton::textColourOffId, juce::Colours::black);
        row->bypassButton->setColour(juce::TextButton::textColourOnId, juce::Colours::black);
        row->bypassButton->addListener(this);
        addAndMakeVisible(*row->bypassButton);

        rows.add(row);
    }

    // If the previously-selected effect no longer exists (removed, or the
    // chain is now empty), fall back to the first remaining effect, or
    // clear the editor entirely if the chain is empty.
    if (count == 0)
    {
        hasSelection = false;
        editorPanel.reset();
    }
    else if (!hasSelection || selectedEffectIndex >= count)
    {
        selectedEffectIndex = 0;
        hasSelection = true;
        editorPanel = std::make_unique<EffectEditorPanel>(
            audioEngine, displayProcessor, selectedEffectIndex,
            displayProcessor.getEffectDescriptors(selectedEffectIndex));
        addAndMakeVisible(*editorPanel);
    }

    resized();
}

void EffectChainPanel::showEditorFor(size_t effectIndex)
{
    hasSelection = true;
    selectedEffectIndex = effectIndex;

    editorPanel = std::make_unique<EffectEditorPanel>(
        audioEngine, displayProcessor, effectIndex,
        displayProcessor.getEffectDescriptors(effectIndex));
    addAndMakeVisible(*editorPanel);

    rebuildEffectList();
}

void EffectChainPanel::showAddEffectMenu()
{
    juce::PopupMenu menu;
    const auto& entries = ags::engine::EffectRegistry::all();

    for (int i = 0; i < static_cast<int>(entries.size()); ++i)
        menu.addItem(i + 1, entries[static_cast<size_t>(i)].displayName);

    menu.showMenuAsync(juce::PopupMenu::Options(), [this, entries](int result)
    {
        if (result <= 0)
            return;

        const auto& chosen = entries[static_cast<size_t>(result - 1)];
        const size_t newIndex = audioEngine.addEffectForAll(chosen.create);
        showEditorFor(newIndex);
    });
}

void EffectChainPanel::buttonClicked(juce::Button* button)
{
    if (button == &addButton)
    {
        showAddEffectMenu();
        return;
    }

    if (button == &removeButton)
    {
        if (!hasSelection)
            return;

        editorPanel.reset();

        const size_t indexToRemove = selectedEffectIndex;
        hasSelection = false;

        audioEngine.removeEffectForAll(selectedEffectIndex);
        rebuildEffectList();
        return;
    }

    for (auto* row : rows)
    {
        if (button == row->nameButton.get())
        {
            showEditorFor(row->effectIndex);
            return;
        }

        if (button == row->bypassButton.get())
        {
            const bool currentlyBypassed = displayProcessor.isEffectBypassed(row->effectIndex);
            audioEngine.setBypassedForAll(row->effectIndex, !currentlyBypassed);
            rebuildEffectList();
            return;
        }
    }
}

void EffectChainPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xffe0e0e0));
    g.setColour(juce::Colours::black);
    g.setFont(20.0f);
    g.drawText("Effects Chain", getLocalBounds().removeFromTop(toolbarHeight),
               juce::Justification::centred);

    g.setColour(juce::Colours::darkgrey);
    g.drawLine(static_cast<float>(listWidth), static_cast<float>(toolbarHeight),
               static_cast<float>(listWidth), static_cast<float>(getHeight()), 2.0f);
}

void EffectChainPanel::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(toolbarHeight);

    auto listArea = bounds.removeFromLeft(listWidth);

    auto toolbarArea = listArea.removeFromTop(toolbarHeight);
    addButton.setBounds(toolbarArea.removeFromLeft(40).reduced(4));
    removeButton.setBounds(toolbarArea.removeFromLeft(40).reduced(4));

    for (auto* row : rows)
    {
        auto rowBounds = listArea.removeFromTop(rowHeight);
        row->bypassButton->setBounds(rowBounds.removeFromRight(bypassButtonWidth).reduced(2));
        row->nameButton->setBounds(rowBounds.reduced(2));
    }

    if (editorPanel != nullptr)
        editorPanel->setBounds(bounds.reduced(8));
}

}