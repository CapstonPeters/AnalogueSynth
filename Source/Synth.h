#pragma once

// JUCE module includes
#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "SynthVoice.h"
#include "ModMatrix.h"
#include "PluginProcessor.h"

class Synth  : public juce::Synthesiser
{
public:
    Synth(juce::AudioProcessorValueTreeState& apvts);
    ~Synth() override = default;
    
    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void release();
    void updateParameters(juce::AudioProcessorValueTreeState& apvts);
    void setModMatrix(ModMatrix* mm) { modMatrix = mm; }
    void renderNextBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi, int startSample, int numSamples);
    
private:
    ModMatrix* modMatrix = nullptr;
    juce::AudioProcessorValueTreeState* params = nullptr;
    int currentPolyphony = 8;
    int currentUnisonVoices = 1;
    float currentUnisonDetune = 10.0f;
    double sampleRate = 44100.0;
    
    void allocateVoices(int numVoices, int unisonVoices, float unisonDetune);
};

struct SynthRenderingContext
{
    juce::AudioBuffer<float>& buffer;
    juce::MidiBuffer& midi;
    int startSample;
    int numSamples;
};