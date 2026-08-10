//
// Created by Yohanes Turnip on 2026-08-09.
//

#ifndef AUDIOGAUSSIANSPLATTER_MANIFOLDVIEWPORT_H
#define AUDIOGAUSSIANSPLATTER_MANIFOLDVIEWPORT_H
#include <juce_gui_basics/juce_gui_basics.h>

namespace ags::app
{
    // A plain component whose bounds define the 3D viewport's on-screen
    // area. juce::OpenGLContext attaches to this, not the whole editor,
    // so the viewport can occupy just its mockup quadrant.
    class ManifoldViewport final : public juce::Component
    {
    public:
        void paint(juce::Graphics&) override {}
    };
}
#endif //AUDIOGAUSSIANSPLATTER_MANIFOLDVIEWPORT_H
