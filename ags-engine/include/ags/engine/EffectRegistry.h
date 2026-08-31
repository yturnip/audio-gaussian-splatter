//
// Created by Yohanes Turnip on 2026-08-16.
//
// Single point of registration for every available EffectProcessor.
// This is the ONLY file that needs to #include individual effect
// headers (SpiralDelayProcessor.h, SpiralTremoloProcessor.h, etc).
//
// To add a brand new effect once its spiral-dsp raw class exists:
//   1. Write a thin EffectProcessor wrapper for it (see
//      SpiralDelayProcessor.h / SpiralTremoloProcessor.h as templates).
//   2. #include its header here.
//   3. Add one line to the `all()` list below.
// AgsAudioProcessor and the Effects Chain GUI never need to change -
// both just iterate EffectRegistry::all() and ask each entry for its
// name/parameters via the EffectProcessor interface itself.
//

#ifndef AUDIOGAUSSIANSPLATTER_EFFECTREGISTRY_H
#define AUDIOGAUSSIANSPLATTER_EFFECTREGISTRY_H

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "ags/engine/EffectProcessor.h"
#include "ags/engine/SpiralDelayProcessor.h"
#include "ags/engine/SpiralTremoloProcessor.h"
#include "ags/engine/SpiralWahWahProcessor.h"
#include "ags/engine/SpiralRingModProcessor.h"
#include "ags/engine/SpiralFlangerProcessor.h"
#include "ags/engine/SpiralVibratoProcessor.h"
#include "ags/engine/SpiralDistortionProcessor.h"
#include "ags/engine/SpiralCompressorProcessor.h"
#include "ags/engine/SpiralExpanderGateProcessor.h"
#include "ags/engine/SpiralPhaseVocoderProcessor.h"

namespace ags::engine
{
    struct EffectRegistryEntry
    {
        std::string id;
        std::string displayName;
        std::function<std::unique_ptr<EffectProcessor>()> create;
    };

    class EffectRegistry
    {
    public:
        // The full list of effects available to be added to any splat's
        // chain. Order here is display order in a future GUI's "add
        // effect" list.
        static const std::vector<EffectRegistryEntry>& all()
        {
            static const std::vector<EffectRegistryEntry> entries = {
                { "delay", "Delay",  [] { return std::make_unique<SpiralDelayProcessor>(); } },
                { "tremolo", "Tremolo", [] { return std::make_unique<SpiralTremoloProcessor>(); } },
                { "wahwah",      "Wah-Wah",       [] { return std::make_unique<SpiralWahWahProcessor>(); } },
                { "ringmod",     "Ring Mod",      [] { return std::make_unique<SpiralRingModProcessor>(); } },
                { "flanger",     "Flanger",       [] { return std::make_unique<SpiralFlangerProcessor>(); } },
                { "vibrato",     "Vibrato",       [] { return std::make_unique<SpiralVibratoProcessor>(); } },
                { "distortion",  "Distortion",    [] { return std::make_unique<SpiralDistortionProcessor>(); } },
                { "compressor",  "Compressor",    [] { return std::make_unique<SpiralCompressorProcessor>(); } },
                { "expandergate","Expander/Gate", [] { return std::make_unique<SpiralExpanderGateProcessor>(); } },
                { "robotvocoder",   "Robot Vocoder",   [] { return std::make_unique<SpiralRobotVocoderProcessor>(); } },
                { "whispervocoder", "Whisper Vocoder", [] { return std::make_unique<SpiralWhisperVocoderProcessor>(); } },
                { "pitchshifter",   "Pitch Shifter",   [] { return std::make_unique<SpiralPitchShifterVocoderProcessor>(); } },
            };
            return entries;
        }

        // Convenience: create by id (e.g. "delay"). Returns nullptr if
        // no entry matches - callers should check before using.
        static std::unique_ptr<EffectProcessor> create(const std::string& id)
        {
            for (const auto& entry : all())
            {
                if (entry.id == id)
                    return entry.create();
            }
            return nullptr;
        }
    };
}
#endif //AUDIOGAUSSIANSPLATTER_EFFECTREGISTRY_H
