//
// Created by Yohanes Turnip on 2026-08-09.
//

#ifndef AUDIOGAUSSIANSPLATTER_ROTATIONPAD_H
#define AUDIOGAUSSIANSPLATTER_ROTATIONPAD_H
#include <juce_gui_basics/juce_gui_basics.h>

namespace ags::app
{
    // A 2D XY-pad matching the reference Slider2D control: the puck's
    // absolute position within the pad maps directly to yaw/pitch,
    // not drag deltas — matching the mockup's "manifold rotation 2D slider".
    class RotationPad final : public juce::Component
    {
    public:
        class Listener
        {
        public:
            virtual ~Listener() = default;
            // normalizedX/Y are both in [0, 1], (0,0) = top-left of pad.
            virtual void rotationPadChanged(float normalizedX, float normalizedY) = 0;
        };

        void addListener(Listener* listener) { listeners.add(listener); }
        void removeListener(Listener* listener) { listeners.remove(listener); }

        void paint(juce::Graphics& g) override;
        void resized() override {}

        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;

        // Lets the editor sync the puck position from host-automated
        // parameter values without feeding back into the parameters.
        void setNormalizedPosition(float normalizedX, float normalizedY);

    private:
        void updateFromMouse(const juce::MouseEvent& event);

        juce::ListenerList<Listener> listeners;
        float puckX { 0.5f };
        float puckY { 0.5f };

        static constexpr float puckRadius = 6.0f;
    };
}
#endif //AUDIOGAUSSIANSPLATTER_ROTATIONPAD_H
