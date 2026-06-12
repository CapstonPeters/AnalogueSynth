#pragma once

#ifndef JucePlugin_Name
#define JucePlugin_Name "AnalogSynth"
#endif

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class AnalogSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    AnalogSynthAudioProcessor()
        : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    {
    }

    ~AnalogSynthAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        // Initialize synth here
        juce::ignoreUnused (sampleRate, samplesPerBlock);
    }

    void releaseResources() override {}
    
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override
    {
        juce::ScopedNoDenormals noDenormals;
        buffer.clear();
        
        // Simple test: generate sine wave for each MIDI note
        for (const auto metadata : midiMessages)
        {
            const auto msg = metadata.getMessage();
            if (msg.isNoteOn())
            {
                // Generate a simple test tone
                const float freq = juce::MidiMessage::getMidiNoteInHertz(msg.getNoteNumber());
                const float level = msg.getVelocity() / 127.0f * 0.1f;
                
                // Just fill first 100 samples with a test tone
                for (int i = 0; i < juce::jmin(100, buffer.getNumSamples()); ++i)
                {
                    const float t = float(i) / float(getSampleRate());
                    const float sample = std::sin(2.0f * juce::MathConstants<float>::pi * freq * t) * level;
                    buffer.addSample(0, metadata.samplePosition + i, sample);
                    buffer.addSample(1, metadata.samplePosition + i, sample);
                }
            }
        }
    }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override
    {
        juce::ignoreUnused (destData);
    }

    void setStateInformation (const void* data, int sizeInBytes) override
    {
        juce::ignoreUnused (data, sizeInBytes);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogSynthAudioProcessor)
};