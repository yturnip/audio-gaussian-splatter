//
// Created by Yohanes Turnip on 2026-08-16.
//

#include "EffectEditorPanel.h"

namespace ags::app
{
namespace
{
constexpr const char* kBindingNames[] = { "None", "Density", "Eccentricity", "Sh Color", "Curvature" };
constexpr int kNumBindingOptions = 5;
}

ags::params::GMMAttribute EffectEditorPanel::comboIndexToAttribute(int comboBoxIndex)
{
    switch (comboBoxIndex)
    {
        case 0: return ags::params::GMMAttribute::None;
        case 1: return ags::params::GMMAttribute::Density;
        case 2: return ags::params::GMMAttribute::Eccentricity;
        case 3: return ags::params::GMMAttribute::ShColor;
        case 4: return ags::params::GMMAttribute::Curvature;
        default: return ags::params::GMMAttribute::None;
    }
}

int EffectEditorPanel::attributeToComboIndex(ags::params::GMMAttribute attribute)
{
    switch (attribute)
    {
        case ags::params::GMMAttribute::None: return 0;
        case ags::params::GMMAttribute::Density: return 1;
        case ags::params::GMMAttribute::Eccentricity: return 2;
        case ags::params::GMMAttribute::ShColor: return 3;
        case ags::params::GMMAttribute::Curvature: return 4;
        default: return 0;
    }
}

EffectEditorPanel::EffectEditorPanel(ags::engine::SplatAudioProcessor& splatProcessor,
                                      size_t effectIndex,
                                      std::vector<ags::engine::EffectParameterDescriptor> descriptors)
    : processor(splatProcessor),
      effectIdx(effectIndex),
      paramDescriptors(std::move(descriptors))
{
    for (const auto& descriptor : paramDescriptors)
    {
        auto* row = new ParamRow();
        row->descriptor = descriptor;

        row->nameLabel.setText(descriptor.name, juce::dontSendNotification);
        row->nameLabel.setJustificationType(juce::Justification::left);
        addAndMakeVisible(row->nameLabel);

        row->valueSlider.setRange(descriptor.minValue, descriptor.maxValue);
        row->valueSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        row->valueSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, rowHeight);
        row->valueSlider.addListener(this);
        addAndMakeVisible(row->valueSlider);

        for (int i = 0; i < kNumBindingOptions; ++i)
            row->bindingCombo.addItem(kBindingNames[i], i + 1);
        row->bindingCombo.addListener(this);
        addAndMakeVisible(row->bindingCombo);

        refreshRowFromState(*row);

        rows.add(row);
    }
}

EffectEditorPanel::~EffectEditorPanel()
{
    for (auto* row : rows)
    {
        row->valueSlider.removeListener(this);
        row->bindingCombo.removeListener(this);
    }
}

void EffectEditorPanel::refreshRowFromState(ParamRow& row)
{
    const auto view = processor.getParameterSlotView(effectIdx, row.descriptor.paramId);
    const bool isBound = view.binding.isActive();

    row.bindingCombo.setSelectedItemIndex(attributeToComboIndex(view.binding.attribute),
                                           juce::dontSendNotification);

    row.valueSlider.setEnabled(!isBound);
    if (!isBound)
        row.valueSlider.setValue(view.currentValue, juce::dontSendNotification);
}

void EffectEditorPanel::sliderValueChanged(juce::Slider* slider)
{
    for (auto* row : rows)
    {
        if (slider != &row->valueSlider)
            continue;

        // Only reachable while unbound -- the slider is disabled while
        // bound, so JUCE won't fire this from user interaction in that case.
        processor.setParameterValue(effectIdx, row->descriptor.paramId,
                                     static_cast<float>(slider->getValue()));
        return;
    }
}

void EffectEditorPanel::comboBoxChanged(juce::ComboBox* comboBox)
{
    for (auto* row : rows)
    {
        if (comboBox != &row->bindingCombo)
            continue;

        const auto attribute = comboIndexToAttribute(comboBox->getSelectedItemIndex());
        processor.setParameterBinding(effectIdx, row->descriptor.paramId,
                                       ags::params::GMMBinding{ attribute, false });

        refreshRowFromState(*row);
        return;
    }
}

void EffectEditorPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xfff0f0f0));
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds(), 1);

    g.setColour(juce::Colours::black);
    g.setFont(16.0f);
    g.drawText("Tremolo", getLocalBounds().removeFromTop(rowHeight).reduced(4),
               juce::Justification::centredLeft);
}

void EffectEditorPanel::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(rowHeight);

    for (auto* row : rows)
    {
        auto rowBounds = bounds.removeFromTop(rowHeight);

        row->nameLabel.setBounds(rowBounds.removeFromLeft(80));
        row->bindingCombo.setBounds(rowBounds.removeFromRight(comboWidth));
        row->valueSlider.setBounds(rowBounds.reduced(4, 0));
    }
}
}