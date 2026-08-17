#include "AgsAudioProcessor.h"
#include "AgsAudioProcessorEditor.h"
#include "ags/engine/EffectRegistry.h"
#include "ags/params/EffectParameter.h"
#include "ags/params/GMMBinding.h"

AgsAudioProcessor::AgsAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    ags::manifold::GeneratorConfig config;
    ags::manifold::SphereBranchingGenerator generator;
    manifold = generator.generate(config);

    for (size_t i = 0; i < manifold.size(); ++i)
    {
        auto splatProcessor = std::make_unique<ags::engine::SplatAudioProcessor>();

        auto tremoloEffect = ags::engine::EffectRegistry::create("tremolo");
        const auto descriptors = tremoloEffect->getParameterDescriptors();
        const auto tremoloIndex = splatProcessor->addEffect(std::move(tremoloEffect));

        for (const auto& descriptor : descriptors)
        {
            const auto binding = (descriptor.name == "Depth")
            ? ags::params::GMMBinding{ ags::params::GMMAttribute::Density, false } :
            ags::params::GMMBinding{ ags::params::GMMAttribute::None, false };

            splatProcessor->addParameterSlot(
                tremoloIndex,
                descriptor.paramId,
                std::make_unique<ags::params::EffectParameter>(
                    descriptor.minValue, descriptor.maxValue, descriptor.defaultValue),
                    binding);
        }

        audioEngine.addSplatProcessor(std::move(splatProcessor));
    }

    addParameter(rotationYaw = new juce::AudioParameterFloat(
        "rotationYaw", "Manifold Yaw",
        -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, 0.0f));

    addParameter(rotationPitch = new juce::AudioParameterFloat(
        "rotationPitch", "Manifold Pitch",
        -juce::MathConstants<float>::pi, juce::MathConstants<float>::pi, 0.0f));
}

void AgsAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    audioEngine.setSampleRate(static_cast<float>(sampleRate));
    audioEngine.reset();

    perSplatBuffer.setSize(static_cast<int>(manifold.size()), samplesPerBlock);
}

void AgsAudioProcessor::releaseResources() {}

void AgsAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    ags::manifold::RotationAngles angles;
    angles.yawRadians = rotationYaw->get();
    angles.pitchRadians = rotationPitch->get();
    audioEngine.setRotation(angles);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels == 0 || manifold.size() == 0)
        return;
    
    const auto* inputChannel = buffer.getReadPointer(0);

    audioEngine.processBlock(inputChannel, numSamples, manifold, perSplatBuffer);

    const size_t numSplatChannels = manifold.size();
    const float scale = numSplatChannels > 0
        ? 1.0f / static_cast<float>(numSplatChannels)
        : 1.0f;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* out = buffer.getWritePointer(ch);
        std::fill(out, out + numSamples, 0.0f);

        for (size_t splatIdx = 0; splatIdx < numSplatChannels; ++splatIdx)
        {
            const auto* splatChannel = perSplatBuffer.getReadPointer(static_cast<int>(splatIdx));
            for (int i = 0; i < numSamples; ++i)
                out[i] += splatChannel[i] * scale;
        }
    }
}

juce::AudioProcessorEditor* AgsAudioProcessor::createEditor()
{
    return new AgsAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AgsAudioProcessor();
}
