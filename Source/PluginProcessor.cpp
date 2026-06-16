#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>

//==============================================================================
AnalogSynthAudioProcessor::AnalogSynthAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // Initialize 8 voices
    for (int i = 0; i < 8; ++i)
        voices.emplace_back();
}

void AnalogSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    for (auto& v : voices) v.prepare(sampleRate);
    testToneOsc.prepare(sampleRate);
    testToneOsc.setFrequency(440.0f); // A4
    juce::ignoreUnused (samplesPerBlock);
}

void AnalogSynthAudioProcessor::releaseResources() {}

bool AnalogSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void AnalogSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // DIAGNOSTIC: Log MIDI activity EVERY BLOCK
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            DBG("MIDI NoteOn: note=" << msg.getNoteNumber() << " vel=" << msg.getVelocity());
            for (auto& v : voices)
            {
                if (!v.isActive())
                {
                    v.startNote(msg.getNoteNumber(), msg.getVelocity());
                    break;
                }
            }
        }
        else if (msg.isNoteOff())
        {
            DBG("MIDI NoteOff: note=" << msg.getNoteNumber());
            for (auto& v : voices)
            {
                if (v.isActive() && v.getNote() == msg.getNoteNumber())
                {
                    v.stopNote();
                    break;
                }
            }
        }
    }

    // Process audio
    const int numSamples = buffer.getNumSamples();
    static int s_blockCount = 0;
    if (++s_blockCount <= 10 || testToneActive.load()) {
        DBG("processBlock #" << s_blockCount << ": numSamples=" << numSamples
            << " channels=" << buffer.getNumChannels()
            << " testToneActive=" << testToneActive.load()
            << " testToneOscActive=" << testToneOsc.isActive());
    }

    for (int i = 0; i < numSamples; ++i)
    {
        float sumL = 0, sumR = 0;
        for (auto& v : voices)
        {
            if (v.isActive())
            {
                float s = v.process() * 3.0f;  // Boost gain for testing
                sumL += s;
                sumR += s;
            }
        }
        // Add test tone if active - SAME GAIN AS VOICES
        if (testToneActive.load())
        {
            // Ensure oscillator is running
            if (!testToneOsc.isActive())
                testToneOsc.noteOn();

            float t = testToneOsc.process() * 3.0f;
            sumL += t;
            sumR += t;

            if (s_blockCount <= 5 && i == 0) {
                DBG("Test tone sample 0: " << t << " phase=" << testToneOsc.getPhase());
            }
        }
        // Force write to both channels
        if (buffer.getNumChannels() > 0) buffer.addSample(0, i, sumL);
        if (buffer.getNumChannels() > 1) buffer.addSample(1, i, sumR);
    }
}

juce::AudioProcessorEditor* AnalogSynthAudioProcessor::createEditor() { return new AnalogSynthAudioProcessorEditor (*this); }

void AnalogSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData) { juce::ignoreUnused (destData); }
void AnalogSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes) { juce::ignoreUnused (data, sizeInBytes); }