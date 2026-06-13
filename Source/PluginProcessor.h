#pragma once

#ifndef JucePlugin_Name
#define JucePlugin_Name "AnalogSynth"
#endif

#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <cmath>

//==============================================================================
// Simple sine oscillator for testing
class TestOscillator
{
public:
    void prepare(double sampleRate) { sr = sampleRate; }
    void setFrequency(float f) { freq = f; phaseInc = f / sr * 2.0 * juce::MathConstants<double>::pi; }
    void noteOn() { phase = 0; active = true; }
    void noteOff() { active = false; }
    void setActive(bool a) { active = a; if (a) phase = 0; }
    bool isActive() const { return active; }
    float process()
    {
        if (!active) return 0.0f;
        float sample = std::sin(phase) * 0.3f;
        phase += phaseInc;
        if (phase > 2.0 * juce::MathConstants<double>::pi) phase -= 2.0 * juce::MathConstants<double>::pi;
        return sample;
    }
private:
    double sr = 44100;
    double phase = 0;
    double phaseInc = 0;
    float freq = 440;
    bool active = false;
};

//==============================================================================
// Simple voice with one oscillator
class TestVoice
{
public:
    void prepare(double sampleRate) { osc.prepare(sampleRate); }
    void startNote(int midiNote, float velocity)
    {
        osc.setFrequency(juce::MidiMessage::getMidiNoteInHertz(midiNote));
        osc.noteOn();
        level = velocity / 127.0f;
        note = midiNote;
    }
    void stopNote() { osc.noteOff(); }
    bool isActive() const { return osc.isActive(); }
    float process() { return osc.process() * level; }
    int getNote() const { return note; }
private:
    TestOscillator osc;
    float level = 0;
    int note = -1;
};

//==============================================================================
class AnalogSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    AnalogSynthAudioProcessor();
    ~AnalogSynthAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    // Test tone control (for debugging audio path)
    void setTestToneActive(bool active) { testToneActive.store(active); if (active) testToneOsc.setActive(true); else testToneOsc.setActive(false); }
    bool isTestToneActive() const { return testToneActive.load(); }

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    std::vector<TestVoice> voices;
    double currentSampleRate = 44100;
    TestOscillator testToneOsc;
    std::atomic<bool> testToneActive{false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogSynthAudioProcessor)
};