#include "Synth.h"
#include "SynthVoice.h"

Synth::Synth(juce::AudioProcessorValueTreeState& apvts) : params(&apvts)
{
    // Initial voice allocation
    allocateVoices(8, 1, 10.0f);
}

void Synth::prepare(double sr, int samplesPerBlock, int numChannels)
{
    sampleRate = sr;
    for (auto* voice : voices)
        if (auto* sv = dynamic_cast<SynthVoice*>(voice))
            sv->prepare(sr, samplesPerBlock);
}

void Synth::release()
{
    for (auto* voice : voices)
        if (auto* sv = dynamic_cast<SynthVoice*>(voice))
            sv->release();
}

void Synth::updateParameters(juce::AudioProcessorValueTreeState& apvts)
{
    int newPoly = (int)*apvts.getRawParameterValue(ParameterID::polyVoices);
    int newUnison = (int)*apvts.getRawParameterValue(ParameterID::unisonVoices);
    float newDetune = *apvts.getRawParameterValue(ParameterID::unisonDetune);
    
    if (newPoly != currentPolyphony || newUnison != currentUnisonVoices || newDetune != currentUnisonDetune) {
        allocateVoices(newPoly, newUnison, newDetune);
    }
    
    // Update all voices
    for (auto* voice : voices)
        if (auto* sv = dynamic_cast<SynthVoice*>(voice))
            sv->updateParameters(apvts);
}

void Synth::allocateVoices(int numVoices, int unisonVoices, float unisonDetune)
{
    currentPolyphony = numVoices;
    currentUnisonVoices = unisonVoices;
    currentUnisonDetune = unisonDetune;
    
    clearVoices();
    
    for (int i = 0; i < numVoices; ++i) {
        addVoice(new SynthVoice(modMatrix, unisonVoices, unisonDetune));
    }
}

void Synth::renderNextBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi, int startSample, int numSamples)
{
    juce::Synthesiser::renderNextBlock(buffer, midi, startSample, numSamples);
}