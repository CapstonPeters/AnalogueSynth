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

    // DIAGNOSTIC: Log MIDI activity
    int midiEventCount = 0;
    for (const auto metadata : midiMessages)
    {
        ++midiEventCount;
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            DBG("MIDI NoteOn: note=" << msg.getNoteNumber() << " vel=" << msg.getVelocity());
            // Find free voice
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
            // Find voice playing this note
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
    static int s_logCount = 0;
    if (++s_logCount % 1000 == 0) // Log every ~1000 blocks
    {
        DBG("processBlock: midiEvents=" << midiEventCount << " activeVoices=" 
            << std::count_if(voices.begin(), voices.end(), [](auto& v){ return v.isActive(); }));
    }

    // Process audio
    const int numSamples = buffer.getNumSamples();
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
        buffer.addSample(0, i, sumL);
        buffer.addSample(1, i, sumR);
    }
}

juce::AudioProcessorEditor* AnalogSynthAudioProcessor::createEditor() { return new AnalogSynthAudioProcessorEditor (*this); }

void AnalogSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData) { juce::ignoreUnused (destData); }
void AnalogSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes) { juce::ignoreUnused (data, sizeInBytes); }