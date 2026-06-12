#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
// Simple sine oscillator for testing
class TestOscillator {
public:
    void prepare(double sampleRate) { sr = sampleRate; }
    void setFrequency(float f) { freq = f; phaseInc = f / sr * 2.0 * juce::MathConstants<double>::pi; }
    void noteOn() { phase = 0; active = true; }
    void noteOff() { active = false; }
    bool isActive() const { return active; }
    float process() {
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
class TestVoice {
public:
    void prepare(double sampleRate) { osc.prepare(sampleRate); }
    void startNote(int midiNote, float velocity) {
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
AnalogSynthAudioProcessor::AnalogSynthAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    // Initialize 8 voices
    for (int i = 0; i < 8; ++i)
        voices.emplace_back();
}

AnalogSynthAudioProcessor::~AnalogSynthAudioProcessor() = default;

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

    // Handle MIDI
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
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

    // Process audio
    const int numSamples = buffer.getNumSamples();
    for (int i = 0; i < numSamples; ++i)
    {
        float sumL = 0, sumR = 0;
        for (auto& v : voices)
        {
            if (v.isActive())
            {
                float s = v.process();
                sumL += s;
                sumR += s;
            }
        }
        buffer.addSample(0, i, sumL);
        buffer.addSample(1, i, sumR);
    }
}

juce::AudioProcessorEditor* AnalogSynthAudioProcessor::createEditor() { return new AnalogSynthAudioProcessorEditor (*this); }
bool AnalogSynthAudioProcessor::hasEditor() const { return true; }
const juce::String AnalogSynthAudioProcessor::getName() const { return JucePlugin_Name; }
bool AnalogSynthAudioProcessor::acceptsMidi() const { return true; }
bool AnalogSynthAudioProcessor::producesMidi() const { return false; }
double AnalogSynthAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int AnalogSynthAudioProcessor::getNumPrograms() { return 1; }
int AnalogSynthAudioProcessor::getCurrentProgram() { return 0; }
void AnalogSynthAudioProcessor::setCurrentProgram (int) {}
const juce::String AnalogSynthAudioProcessor::getProgramName (int) { return {}; }
void AnalogSynthAudioProcessor::changeProgramName (int, const juce::String&) {}

void AnalogSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData) { juce::ignoreUnused (destData); }
void AnalogSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes) { juce::ignoreUnused (data, sizeInBytes); }