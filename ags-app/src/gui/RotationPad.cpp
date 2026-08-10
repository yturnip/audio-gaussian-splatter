//
// Created by Yohanes Turnip on 2026-08-09.
//
#include "RotationPad.h"

namespace ags::app
{
    void RotationPad::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();

        g.setColour(juce::Colours::lightgrey);
        g.fillRect(bounds);

        g.setColour(juce::Colours::black);
        g.drawRect(bounds, 2.0f);

        // Crosshair through the center, matching the mockup's plain style.
        g.drawLine(bounds.getX(), bounds.getCentreY(), bounds.getRight(), bounds.getCentreY());
        g.drawLine(bounds.getCentreX(), bounds.getY(), bounds.getCentreX(), bounds.getBottom());

        const auto puckPos = juce::Point<float>(
            bounds.getX() + puckX * bounds.getWidth(),
            bounds.getY() + puckY * bounds.getHeight());

        g.setColour(juce::Colours::darkorange);
        g.fillEllipse(puckPos.x - puckRadius, puckPos.y - puckRadius, puckRadius * 2.0f, puckRadius * 2.0f);
    }

    void RotationPad::mouseDown(const juce::MouseEvent& event)
    {
        updateFromMouse(event);
    }

    void RotationPad::mouseDrag(const juce::MouseEvent& event)
    {
        updateFromMouse(event);
    }

    void RotationPad::updateFromMouse(const juce::MouseEvent& event)
    {
        const auto bounds = getLocalBounds().toFloat();
        if (bounds.getWidth() <= 0.0f || bounds.getHeight() <= 0.0f)
            return;

        puckX = juce::jlimit(0.0f, 1.0f, event.position.x / bounds.getWidth());
        puckY = juce::jlimit(0.0f, 1.0f, event.position.y / bounds.getHeight());

        repaint();
        listeners.call([this](Listener& l) { l.rotationPadChanged(puckX, puckY); });
    }

    void RotationPad::setNormalizedPosition(float normalizedX, float normalizedY)
    {
        puckX = juce::jlimit(0.0f, 1.0f, normalizedX);
        puckY = juce::jlimit(0.0f, 1.0f, normalizedY);
        repaint();
    }
}