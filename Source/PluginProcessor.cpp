#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout AnalogSynthAudioProcessor::createParameterLayout()
{
    FLOG("createParameterLayout: START");
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto addFloat = [&](const char* id, const juce::String& name, float min, float max, float def)
    { params.push_back(std::make_unique<juce::AudioParameterFloat>(id, name, min, max, def)); };
    auto addInt = [&](const char* id, const juce::String& name, int min, int max, int def)
    { params.push_back(std::make_unique<juce::AudioParameterInt>(id, name, min, max, def)); };
    auto addChoice = [&](const char* id, const juce::String& name, const juce::StringArray& choices, int def)
    { params.push_back(std::make_unique<juce::AudioParameterChoice>(id, name, choices, def)); };

    FLOG("createParameterLayout: lambdas created, adding params...");

    // Global
    addFloat(ParamID::masterVolume, "Master Volume", -60.0f, 12.0f, 0.0f);
    addFloat(ParamID::masterTune,   "Master Tune",    -12.0f, 12.0f, 0.0f);
    addInt   (ParamID::polyphony,   "Polyphony",      1, 16, 8);
    addInt   (ParamID::unisonVoices, "Unison Voices", 1, 8, 1);
    addFloat (ParamID::unisonDetune, "Unison Detune", 0.0f, 50.0f, 5.0f);
    addFloat (ParamID::unisonSpread, "Unison Spread", 0.0f, 100.0f, 20.0f);

    FLOG("createParameterLayout: global params added");

    // Oscillator 1
    addChoice(ParamID::osc1Waveform, "Osc 1 Waveform", { "Sine", "Triangle", "Saw", "Square", "Noise" }, 2);
    addFloat(ParamID::osc1Level,      "Osc 1 Level",      0.0f, 1.0f, 1.0f);
    addFloat(ParamID::osc1Pitch,      "Osc 1 Pitch",      -24.0f, 24.0f, 0.0f);
    addFloat(ParamID::osc1Detune,     "Osc 1 Detune",     -100.0f, 100.0f, 0.0f);
    addFloat(ParamID::osc1Pan,        "Osc 1 Pan",        -1.0f, 1.0f, 0.0f);
    addFloat(ParamID::osc1PulseWidth, "Osc 1 Pulse Width", 0.01f, 0.99f, 0.5f);

    // Oscillator 2
    addChoice(ParamID::osc2Waveform, "Osc 2 Waveform", { "Sine", "Triangle", "Saw", "Square", "Noise" }, 2);
    addFloat(ParamID::osc2Level,      "Osc 2 Level",      0.0f, 1.0f, 0.5f);
    addFloat(ParamID::osc2Pitch,      "Osc 2 Pitch",      -24.0f, 24.0f, 0.0f);
    addFloat(ParamID::osc2Detune,     "Osc 2 Detune",     -100.0f, 100.0f, 0.0f);
    addFloat(ParamID::osc2Pan,        "Osc 2 Pan",        -1.0f, 1.0f, 0.0f);
    addFloat(ParamID::osc2PulseWidth, "Osc 2 Pulse Width", 0.01f, 0.99f, 0.5f);

    // Oscillator 3
    addChoice(ParamID::osc3Waveform, "Osc 3 Waveform", { "Sine", "Triangle", "Saw", "Square", "Noise" }, 0);
    addFloat(ParamID::osc3Level,      "Osc 3 Level",      0.0f, 1.0f, 0.0f);
    addFloat(ParamID::osc3Pitch,      "Osc 3 Pitch",      -24.0f, 24.0f, 0.0f);
    addFloat(ParamID::osc3Detune,     "Osc 3 Detune",     -100.0f, 100.0f, 0.0f);
    addFloat(ParamID::osc3Pan,        "Osc 3 Pan",        -1.0f, 1.0f, 0.0f);
    addFloat(ParamID::osc3PulseWidth, "Osc 3 Pulse Width", 0.01f, 0.99f, 0.5f);

    // Mix
    addFloat(ParamID::oscMixBalance12,  "Osc 1/2 Balance", -1.0f, 1.0f, 0.0f);
    addFloat(ParamID::oscMixBalance123, "Osc 12/3 Balance", -1.0f, 1.0f, 0.0f);
    addFloat(ParamID::noiseLevel,       "Noise Level", 0.0f, 1.0f, 0.0f);
    addFloat(ParamID::subLevel,         "Sub Level",   0.0f, 1.0f, 0.0f);

    FLOG("createParameterLayout: osc params added");

    // Filter
    addChoice(ParamID::filterType, "Filter Type", { "LP12", "LP24", "HP12", "HP24", "BP12", "BP24", "Notch" }, 1);
    addFloat(ParamID::filterCutoff,    "Filter Cutoff",    20.0f, 20000.0f, 10000.0f);
    addFloat(ParamID::filterResonance, "Filter Resonance", 0.0f, 1.0f, 0.0f);
    addFloat(ParamID::filterDrive,     "Filter Drive",     0.0f, 10.0f, 0.0f);
    addFloat(ParamID::filterKeyTrack,  "Filter Key Track", -1.0f, 1.0f, 0.5f);
    addFloat(ParamID::filterVelTrack,  "Filter Vel Track", -1.0f, 1.0f, 0.0f);
    addFloat(ParamID::filterEnvAmount, "Filter Env Amount", -1.0f, 1.0f, 0.0f);

    // Amp Envelope
    addFloat(ParamID::ampAttack,  "Amp Attack",  0.001f, 10.0f, 0.01f);
    addFloat(ParamID::ampDecay,   "Amp Decay",   0.001f, 10.0f, 0.1f);
    addFloat(ParamID::ampSustain, "Amp Sustain", 0.0f,   1.0f,  0.7f);
    addFloat(ParamID::ampRelease, "Amp Release", 0.001f, 10.0f, 0.2f);

    // Filter Envelope
    addFloat(ParamID::filtAttack,  "Filt Attack",  0.001f, 10.0f, 0.01f);
    addFloat(ParamID::filtDecay,   "Filt Decay",   0.001f, 10.0f, 0.1f);
    addFloat(ParamID::filtSustain, "Filt Sustain", 0.0f,   1.0f,  0.0f);
    addFloat(ParamID::filtRelease, "Filt Release", 0.001f, 10.0f, 0.2f);

    // LFO 1
    addFloat(ParamID::lfo1Rate,     "LFO 1 Rate",     0.01f, 30.0f, 1.0f);
    addChoice(ParamID::lfo1Waveform, "LFO 1 Waveform", { "Sine", "Triangle", "Saw", "Square", "S&H" }, 0);
    addFloat(ParamID::lfo1Delay,    "LFO 1 Delay",    0.0f, 5.0f, 0.0f);
    addFloat(ParamID::lfo1Fade,     "LFO 1 Fade",     0.0f, 5.0f, 0.0f);

    // LFO 2
    addFloat(ParamID::lfo2Rate,     "LFO 2 Rate",     0.01f, 30.0f, 0.5f);
    addChoice(ParamID::lfo2Waveform, "LFO 2 Waveform", { "Sine", "Triangle", "Saw", "Square", "S&H" }, 1);
    addFloat(ParamID::lfo2Delay,    "LFO 2 Delay",    0.0f, 5.0f, 0.0f);
    addFloat(ParamID::lfo2Fade,     "LFO 2 Fade",     0.0f, 5.0f, 0.0f);

    FLOG("createParameterLayout: env/lfo params added");

    // Mod Matrix (8 slots)
    juce::StringArray modSources = { "None", "LFO 1", "LFO 2", "Amp Env", "Filt Env", "Velocity", "Mod Wheel", "Aftertouch", "Pitch Bend", "Key Track" };
    juce::StringArray modDests = { "None", "Osc 1 Pitch", "Osc 2 Pitch", "Osc 3 Pitch", "All Osc Pitch", "Osc 1 Level", "Osc 2 Level", "Osc 3 Level", "Filter Cutoff", "Filter Resonance", "Filter Drive", "Amp Level", "Pan", "LFO 1 Rate", "LFO 2 Rate" };

    for (int i = 0; i < 8; ++i)
    {
        addChoice(ParamID::modSrc[i], "Mod " + juce::String(i+1) + " Source", modSources, 0);
        addChoice(ParamID::modDst[i], "Mod " + juce::String(i+1) + " Dest",   modDests, 0);
        addFloat(ParamID::modAmt[i],  "Mod " + juce::String(i+1) + " Amount", -1.0f, 1.0f, 0.0f);
    }

    FLOG("createParameterLayout: mod matrix params added, total=" << params.size());
    return { params.begin(), params.end() };
}

//==============================================================================
AnalogSynthAudioProcessor::AnalogSynthAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    , apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    FLOG("AnalogSynthAudioProcessor: Constructor start");
    // Initialize 16 voices max
    for (int i = 0; i < 16; ++i)
        synth.addVoice(new SynthVoice());

    synth.addSound(new SimpleSynthSound());
    FLOG("AnalogSynthAudioProcessor: Constructor done, voices=" << synth.getVoicesArray().size());
}

void AnalogSynthAudioProcessor::updateSynthParamsIfNeeded()
{
    float poly = apvts.getRawParameterValue(ParamID::polyphony)->load();
    if (poly != lastPolyphony)
    {
        lastPolyphony = poly;
        synth.setParams(apvts);
    }
}

void AnalogSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    FLOG("prepareToPlay: sr=" << sampleRate << " block=" << samplesPerBlock);
    synth.setCurrentPlaybackSampleRate(sampleRate);
    for (auto* v : synth.getVoicesArray())
        if (auto* sv = dynamic_cast<SynthVoice*>(v))
            sv->prepare(sampleRate);

    testToneOsc.prepare(sampleRate);
    testToneOsc.setFrequency(440.0f);
    
    // Initial param sync
    updateSynthParamsIfNeeded();
    FLOG("prepareToPlay: done");
}

void AnalogSynthAudioProcessor::releaseResources() {}

bool AnalogSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void AnalogSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    static int processBlockCount = 0;
    static int maxProcessBlockSamples = 0;
    processBlockCount++;
    int totalSamplesThisBlock = buffer.getNumSamples();
    maxProcessBlockSamples = juce::jmax(maxProcessBlockSamples, totalSamplesThisBlock);
    
    // Log first few blocks
    if (processBlockCount <= 3)
    {
        FLOG("processBlock #" << processBlockCount << ": samples=" << totalSamplesThisBlock 
            << " channels=" << buffer.getNumChannels() 
            << " midiEvents=" << midiMessages.getNumEvents());
    }
    
    // Safety: if we've processed too many blocks without audio output, reset
    if (processBlockCount > 100000) processBlockCount = 0;

    buffer.clear();

    // Update synth params only when polyphony changes
    updateSynthParamsIfNeeded();

    // Process MIDI
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            FLOG("MIDI NoteOn: note=" << msg.getNoteNumber() << " vel=" << msg.getVelocity());
        }
        else if (msg.isNoteOff())
        {
            FLOG("MIDI NoteOff: note=" << msg.getNoteNumber());
        }
    }

    // Render
    juce::MidiBuffer midiCopy = midiMessages;
    synth.renderNextBlock(buffer, midiCopy, 0, buffer.getNumSamples());

    // Safety: check output for NaN/Inf
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float sample = buffer.getSample(ch, i);
            if (!std::isfinite(sample)) { buffer.setSample(ch, i, 0.0f); }
        }
    }

    // Test tone (for debugging)
    if (testToneActive.load())
    {
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            if (!testToneOsc.isActive()) testToneOsc.noteOn();
            float t = testToneOsc.process() * 3.0f;
            if (buffer.getNumChannels() > 0) buffer.addSample(0, i, t);
            if (buffer.getNumChannels() > 1) buffer.addSample(1, i, t);
        }
    }
}

juce::AudioProcessorEditor* AnalogSynthAudioProcessor::createEditor()
{
    return new AnalogSynthAudioProcessorEditor (*this);
}

void AnalogSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AnalogSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}