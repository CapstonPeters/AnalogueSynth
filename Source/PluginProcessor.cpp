#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>

//==============================================================================
// SynthVoice implementation
void SynthVoice::updateParams(const SynthParams& p)
{
    // Oscillators
    for (int i = 0; i < 3; ++i)
    {
        oscillators[i].setWaveType(static_cast<Oscillator::WaveType>(p.osc[i].wave));
        oscillators[i].setLevel(p.osc[i].level);
        oscillators[i].setPan(p.osc[i].pan);
        oscillators[i].setUnison(p.osc[i].unison);
        oscillators[i].setDetune(p.osc[i].detune);
        oscillators[i].setPulseWidth(p.osc[i].pulseWidth);
        oscPitchOffsets[i] = p.osc[i].pitch + p.osc[i].fineTune / 100.0f;
    }
    
    // Sub
    subOsc.setWaveType(static_cast<Oscillator::WaveType>(p.sub.wave));
    subOsc.setLevel(p.sub.level);
    subOsc.setPan(0);
    
    // Noise
    noiseOsc.setWaveType(Oscillator::Noise);
    noiseOsc.setLevel(p.noise.level);
    noiseOsc.setPan(0);
    
    // Filter
    filter.setParams(static_cast<LadderFilter::Type>(p.filterType), 
                     p.filterCutoff, p.filterResonance, p.filterDrive, 
                     p.filterKeyTrack, p.filterVelTrack);
    
    // Amp Envelope
    ampEnv.setParams(p.ampAttack, p.ampDecay, p.ampSustain, p.ampRelease);
    ampVelSens = p.ampVelSens;
    
    // Filter Envelope
    filtEnv.setParams(p.filtAttack, p.filtDecay, p.filtSustain, p.filtRelease);
    filtVelSens = p.filtVelSens;
    
    // LFOs
    lfo1.setParams(static_cast<LFO::WaveType>(p.lfo1Wave), p.lfo1Rate, p.lfo1Amount, p.lfo1Delay, p.lfo1Fade);
    lfo2.setParams(static_cast<LFO::WaveType>(p.lfo2Wave), p.lfo2Rate, p.lfo2Amount, p.lfo2Delay, p.lfo2Fade);
}

void SynthVoice::process(float& outL, float& outR)
{
    if (!active) { outL = outR = 0; return; }
    
    // Safety timeout - check at start of each process call
    if (samplesSinceNoteOn > MaxVoiceSamples)
    {
        active = false;
        outL = outR = 0;
        return;
    }
    
    // Process LFOs
    lfo1Out = lfo1.process();
    lfo2Out = lfo2.process();
    
    // Process Envelopes
    ampEnvOut = ampEnv.process();
    filtEnvOut = filtEnv.process();
    
    if (!ampEnv.isActive() && !filtEnv.isActive() && !lfo1.isActive() && !lfo2.isActive())
    {
        // All done, check if oscillators are still running
        bool anyOscActive = false;
        for (auto& osc : oscillators) if (osc.isActive()) anyOscActive = true;
        if (subOsc.isActive()) anyOscActive = true;
        if (noiseOsc.isActive()) anyOscActive = true;
        if (!anyOscActive) { active = false; outL = outR = 0; return; }
    }
    
    // Get modulation values
    float keyTrack = (note - 60) / 12.0f; // Middle C = 0
    float modWheelVal = 0; // Would come from MIDI
    float aftertouchVal = 0;
    float pitchBendVal = 0;
    float exprPedalVal = 1.0f;
    
    // Filter cutoff modulation
    float filtMod = modMatrix.getModulation(ModDest::FilterCutoff, lfo1Out, lfo2Out, ampEnvOut, filtEnvOut, velocity, keyTrack, modWheelVal, aftertouchVal, pitchBendVal, exprPedalVal);
    filtMod += filtEnvOut * synthParams.filtAmount;
    filter.setCutoffMod(filtMod);
    
    // Oscillator pitch modulation (LFO1 -> pitch)
    float pitchMod = modMatrix.getModulation(ModDest::Osc1Pitch, lfo1Out, lfo2Out, ampEnvOut, filtEnvOut, velocity, keyTrack, modWheelVal, aftertouchVal, pitchBendVal, exprPedalVal);
    float pitchMult = std::pow(2.0f, pitchMod / 12.0f);
    
    // Process oscillators
    float oscSumL = 0, oscSumR = 0;
    
    for (int i = 0; i < 3; ++i)
    {
        if (!oscillators[i].isActive()) continue;
        float sample = oscillators[i].process() * pitchMult;
        oscSumL += sample * oscillators[i].getLeftGain();
        oscSumR += sample * oscillators[i].getRightGain();
    }
    
    if (subOsc.isActive())
    {
        float sample = subOsc.process();
        oscSumL += sample * subOsc.getLeftGain();
        oscSumR += sample * subOsc.getRightGain();
    }
    
    if (noiseOsc.isActive())
    {
        float sample = noiseOsc.process();
        oscSumL += sample * noiseOsc.getLeftGain();
        oscSumR += sample * noiseOsc.getRightGain();
    }
    
    // Apply amp envelope
    oscSumL *= ampEnvOut * synthParams.masterGain;
    oscSumR *= ampEnvOut * synthParams.masterGain;
    
    // Process filter
    float filteredL, filteredR;
    filter.processBlock(&oscSumL, &oscSumR, &filteredL, &filteredR, 1);
    
    outL = filteredL;
    outR = filteredR;
    
    // NaN/Inf protection
    if (!std::isfinite(outL)) outL = 0;
    if (!std::isfinite(outR)) outR = 0;
    
    // Hard clip
    outL = std::clamp(outL, -1.0f, 1.0f);
    outR = std::clamp(outR, -1.0f, 1.0f);
    
    // Increment sample counter
    samplesSinceNoteOn++;
}

//==============================================================================
// AnalogSynthAudioProcessor

AnalogSynthAudioProcessor::AnalogSynthAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    , apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    // Initialize 8 voices in synth engine
    synth.prepare(44100.0, 8);
    
    // Test tone
    testToneOsc.prepare(44100.0);
    testToneOsc.setFrequency(440.0f);
    
    // Load initial params
    updateSynthParams();
}

void AnalogSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.prepare(sampleRate, 16); // Max 16 voices
    testToneOsc.prepare(sampleRate);
    testToneOsc.setFrequency(440.0f);
    
    updateSynthParams();
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
    
    const int numSamples = buffer.getNumSamples();
    
    // Update params from APVTS every 64 blocks to avoid overhead
    static int paramUpdateCounter = 0;
    if (++paramUpdateCounter >= 64)
    {
        paramUpdateCounter = 0;
        updateSynthParams();
    }
    
    // Process MIDI
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        const int frame = metadata.samplePosition;
        
        if (msg.isNoteOn())
        {
            synth.noteOn(msg.getNoteNumber(), msg.getVelocity() / 127.0f);
        }
        else if (msg.isNoteOff())
        {
            synth.noteOff(msg.getNoteNumber());
        }
        else if (msg.isController())
        {
            switch (msg.getControllerNumber())
            {
                case 1:  // Mod wheel
                    lastModWheel = msg.getControllerValue() / 127.0f;
                    synth.setModWheel(lastModWheel);
                    break;
                case 11: // Expression
                    lastExprPedal = msg.getControllerValue() / 127.0f;
                    synth.setExprPedal(lastExprPedal);
                    break;
                case 64: // Sustain - handle in voice
                    break;
            }
        }
        else if (msg.isChannelPressure())
        {
            lastAftertouch = msg.getChannelPressureValue() / 127.0f;
            synth.setAftertouch(lastAftertouch);
        }
        else if (msg.isPitchWheel())
        {
            lastPitchBend = msg.getPitchWheelValue() / 8192.0f; // -1 to 1
            synth.setPitchBend(lastPitchBend);
        }
    }
    
    // Process audio
    float* outL = buffer.getWritePointer(0);
    float* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : outL;
    
    if (testToneActive.load())
    {
        // Test tone mode
        if (!testToneOsc.isActive()) testToneOsc.setActive(true);
        
        for (int i = 0; i < numSamples; ++i)
        {
            float t = testToneOsc.process() * 3.0f; // Same gain as voices
            outL[i] += t;
            outR[i] += t;
        }
    }
    else
    {
        // Normal synth mode
        synth.processBlock(outL, outR, numSamples);
    }
}

//==============================================================================
// Parameter Layout
juce::AudioProcessorValueTreeState::ParameterLayout AnalogSynthAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    auto addFloat = [&](const char* id, const char* name, float min, float max, float def, const char* unit = "")
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat>(id, name, juce::NormalisableRange<float>(min, max, 0.001f, 1.0f), def, unit));
    };
    
    auto addInt = [&](const char* id, const char* name, int min, int max, int def)
    {
        params.push_back(std::make_unique<juce::AudioParameterInt>(id, name, min, max, def));
    };
    
    auto addChoice = [&](const char* id, const char* name, const juce::StringArray& choices, int def)
    {
        params.push_back(std::make_unique<juce::AudioParameterChoice>(id, name, choices, def));
    };
    
    // Global
    addFloat(ParameterIDs::masterGain, "Master Gain", 0.0f, 1.0f, 0.5f, "dB");
    addInt(ParameterIDs::polyphony, "Polyphony", 1, 16, 8);
    addFloat(ParameterIDs::pitchBendRange, "Pitch Bend Range", 0.0f, 24.0f, 2.0f, "semitones");
    
    // Oscillator choices
    juce::StringArray oscWaves = {"Sine", "Triangle", "Saw", "Square", "Noise"};
    juce::StringArray subWaves = {"Sine", "Square"};
    juce::StringArray noiseTypes = {"White", "Pink"};
    juce::StringArray filterTypes = {"LP 4-Pole", "LP 2-Pole", "HP 4-Pole", "HP 2-Pole", "Bandpass", "Notch"};
    juce::StringArray lfoWaves = {"Sine", "Triangle", "Saw", "Square", "S&H"};
    
    // Osc 1
    addChoice(ParameterIDs::osc1Wave, "Osc 1 Wave", oscWaves, 2);
    addFloat(ParameterIDs::osc1Level, "Osc 1 Level", 0.0f, 1.0f, 0.7f);
    addFloat(ParameterIDs::osc1Pitch, "Osc 1 Pitch", -24.0f, 24.0f, 0.0f, "semitones");
    addFloat(ParameterIDs::osc1FineTune, "Osc 1 Fine Tune", -50.0f, 50.0f, 0.0f, "cents");
    addFloat(ParameterIDs::osc1Pan, "Osc 1 Pan", -1.0f, 1.0f, 0.0f);
    addInt(ParameterIDs::osc1Unison, "Osc 1 Unison", 1, 8, 1);
    addFloat(ParameterIDs::osc1Detune, "Osc 1 Detune", 0.0f, 50.0f, 0.0f, "cents");
    addFloat(ParameterIDs::osc1PulseWidth, "Osc 1 Pulse Width", 0.01f, 0.99f, 0.5f);
    
    // Osc 2
    addChoice(ParameterIDs::osc2Wave, "Osc 2 Wave", oscWaves, 2);
    addFloat(ParameterIDs::osc2Level, "Osc 2 Level", 0.0f, 1.0f, 0.5f);
    addFloat(ParameterIDs::osc2Pitch, "Osc 2 Pitch", -24.0f, 24.0f, 0.0f, "semitones");
    addFloat(ParameterIDs::osc2FineTune, "Osc 2 Fine Tune", -50.0f, 50.0f, 0.0f, "cents");
    addFloat(ParameterIDs::osc2Pan, "Osc 2 Pan", -1.0f, 1.0f, 0.0f);
    addInt(ParameterIDs::osc2Unison, "Osc 2 Unison", 1, 8, 1);
    addFloat(ParameterIDs::osc2Detune, "Osc 2 Detune", 0.0f, 50.0f, 0.0f, "cents");
    addFloat(ParameterIDs::osc2PulseWidth, "Osc 2 Pulse Width", 0.01f, 0.99f, 0.5f);
    
    // Osc 3
    addChoice(ParameterIDs::osc3Wave, "Osc 3 Wave", oscWaves, 2);
    addFloat(ParameterIDs::osc3Level, "Osc 3 Level", 0.0f, 1.0f, 0.3f);
    addFloat(ParameterIDs::osc3Pitch, "Osc 3 Pitch", -24.0f, 24.0f, 0.0f, "semitones");
    addFloat(ParameterIDs::osc3FineTune, "Osc 3 Fine Tune", -50.0f, 50.0f, 0.0f, "cents");
    addFloat(ParameterIDs::osc3Pan, "Osc 3 Pan", -1.0f, 1.0f, 0.0f);
    addInt(ParameterIDs::osc3Unison, "Osc 3 Unison", 1, 8, 1);
    addFloat(ParameterIDs::osc3Detune, "Osc 3 Detune", 0.0f, 50.0f, 0.0f, "cents");
    addFloat(ParameterIDs::osc3PulseWidth, "Osc 3 Pulse Width", 0.01f, 0.99f, 0.5f);
    
    // Sub
    addChoice(ParameterIDs::subWave, "Sub Wave", subWaves, 0);
    addFloat(ParameterIDs::subLevel, "Sub Level", 0.0f, 1.0f, 0.0f);
    addFloat(ParameterIDs::subPitch, "Sub Pitch", -24.0f, 0.0f, -12.0f, "semitones");
    
    // Noise
    addChoice(ParameterIDs::noiseType, "Noise Type", noiseTypes, 0);
    addFloat(ParameterIDs::noiseLevel, "Noise Level", 0.0f, 1.0f, 0.0f);
    
    // Filter
    addChoice(ParameterIDs::filterType, "Filter Type", filterTypes, 0);
    addFloat(ParameterIDs::filterCutoff, "Filter Cutoff", 20.0f, 20000.0f, 1000.0f, "Hz");
    addFloat(ParameterIDs::filterResonance, "Filter Resonance", 0.0f, 1.0f, 0.0f);
    addFloat(ParameterIDs::filterDrive, "Filter Drive", 0.0f, 1.0f, 0.0f);
    addFloat(ParameterIDs::filterKeyTrack, "Filter Key Track", -1.0f, 1.0f, 0.0f);
    addFloat(ParameterIDs::filterVelTrack, "Filter Vel Track", -1.0f, 1.0f, 0.0f);
    
    // Amp Envelope
    addFloat(ParameterIDs::ampAttack, "Amp Attack", 0.001f, 10.0f, 0.01f, "s");
    addFloat(ParameterIDs::ampDecay, "Amp Decay", 0.001f, 10.0f, 0.3f, "s");
    addFloat(ParameterIDs::ampSustain, "Amp Sustain", 0.0f, 1.0f, 0.7f);
    addFloat(ParameterIDs::ampRelease, "Amp Release", 0.001f, 10.0f, 0.3f, "s");
    addFloat(ParameterIDs::ampVelSens, "Amp Vel Sens", 0.0f, 1.0f, 0.5f);
    
    // Filter Envelope
    addFloat(ParameterIDs::filtAttack, "Filt Attack", 0.001f, 10.0f, 0.01f, "s");
    addFloat(ParameterIDs::filtDecay, "Filt Decay", 0.001f, 10.0f, 0.3f, "s");
    addFloat(ParameterIDs::filtSustain, "Filt Sustain", 0.0f, 1.0f, 0.0f);
    addFloat(ParameterIDs::filtRelease, "Filt Release", 0.001f, 10.0f, 0.3f, "s");
    addFloat(ParameterIDs::filtAmount, "Filt Amount", -1.0f, 1.0f, 0.5f);
    addFloat(ParameterIDs::filtVelSens, "Filt Vel Sens", 0.0f, 1.0f, 0.0f);
    
    // LFO 1
    addChoice(ParameterIDs::lfo1Wave, "LFO 1 Wave", lfoWaves, 0);
    addFloat(ParameterIDs::lfo1Rate, "LFO 1 Rate", 0.01f, 20.0f, 1.0f, "Hz");
    addFloat(ParameterIDs::lfo1Amount, "LFO 1 Amount", 0.0f, 1.0f, 0.0f);
    addFloat(ParameterIDs::lfo1Delay, "LFO 1 Delay", 0.0f, 5.0f, 0.0f, "s");
    addFloat(ParameterIDs::lfo1Fade, "LFO 1 Fade", 0.0f, 5.0f, 0.0f, "s");
    
    // LFO 2
    addChoice(ParameterIDs::lfo2Wave, "LFO 2 Wave", lfoWaves, 1);
    addFloat(ParameterIDs::lfo2Rate, "LFO 2 Rate", 0.01f, 20.0f, 5.0f, "Hz");
    addFloat(ParameterIDs::lfo2Amount, "LFO 2 Amount", 0.0f, 1.0f, 0.0f);
    addFloat(ParameterIDs::lfo2Delay, "LFO 2 Delay", 0.0f, 5.0f, 0.0f, "s");
    addFloat(ParameterIDs::lfo2Fade, "LFO 2 Fade", 0.0f, 5.0f, 0.0f, "s");
    
    // Mod Matrix (8 slots - simplified as individual params for now)
    // In a full implementation, these would be structured differently
    // For now we'll skip detailed mod matrix params and use defaults
    
    return { params.begin(), params.end() };
}

void AnalogSynthAudioProcessor::updateSynthParams()
{
    auto getF = [&](const char* id) { return apvts.getRawParameterValue(id)->load(); };
    auto getI = [&](const char* id) { return apvts.getRawParameterValue(id)->load(); };
    
    // Global
    synthParams.masterGain = getF(ParameterIDs::masterGain);
    synthParams.polyphony = static_cast<int>(getI(ParameterIDs::polyphony));
    synthParams.pitchBendRange = getF(ParameterIDs::pitchBendRange);
    
    // Oscillators
    for (int i = 0; i < 3; ++i)
    {
        const char* prefix[] = {ParameterIDs::osc1Wave, ParameterIDs::osc2Wave, ParameterIDs::osc3Wave};
        const char* level[] = {ParameterIDs::osc1Level, ParameterIDs::osc2Level, ParameterIDs::osc3Level};
        const char* pitch[] = {ParameterIDs::osc1Pitch, ParameterIDs::osc2Pitch, ParameterIDs::osc3Pitch};
        const char* fine[] = {ParameterIDs::osc1FineTune, ParameterIDs::osc2FineTune, ParameterIDs::osc3FineTune};
        const char* pan[] = {ParameterIDs::osc1Pan, ParameterIDs::osc2Pan, ParameterIDs::osc3Pan};
        const char* unison[] = {ParameterIDs::osc1Unison, ParameterIDs::osc2Unison, ParameterIDs::osc3Unison};
        const char* detune[] = {ParameterIDs::osc1Detune, ParameterIDs::osc2Detune, ParameterIDs::osc3Detune};
        const char* pw[] = {ParameterIDs::osc1PulseWidth, ParameterIDs::osc2PulseWidth, ParameterIDs::osc3PulseWidth};
        
        synthParams.osc[i].wave = static_cast<int>(getI(prefix[i]));
        synthParams.osc[i].level = getF(level[i]);
        synthParams.osc[i].pitch = getF(pitch[i]);
        synthParams.osc[i].fineTune = getF(fine[i]);
        synthParams.osc[i].pan = getF(pan[i]);
        synthParams.osc[i].unison = static_cast<int>(getI(unison[i]));
        synthParams.osc[i].detune = getF(detune[i]);
        synthParams.osc[i].pulseWidth = getF(pw[i]);
    }
    
    // Sub
    synthParams.sub.wave = static_cast<int>(getI(ParameterIDs::subWave));
    synthParams.sub.level = getF(ParameterIDs::subLevel);
    synthParams.sub.pitch = getF(ParameterIDs::subPitch);
    
    // Noise
    synthParams.noise.type = static_cast<int>(getI(ParameterIDs::noiseType));
    synthParams.noise.level = getF(ParameterIDs::noiseLevel);
    
    // Filter
    synthParams.filterType = static_cast<int>(getI(ParameterIDs::filterType));
    synthParams.filterCutoff = getF(ParameterIDs::filterCutoff);
    synthParams.filterResonance = getF(ParameterIDs::filterResonance);
    synthParams.filterDrive = getF(ParameterIDs::filterDrive);
    synthParams.filterKeyTrack = getF(ParameterIDs::filterKeyTrack);
    synthParams.filterVelTrack = getF(ParameterIDs::filterVelTrack);
    
    // Amp Envelope
    synthParams.ampAttack = getF(ParameterIDs::ampAttack);
    synthParams.ampDecay = getF(ParameterIDs::ampDecay);
    synthParams.ampSustain = getF(ParameterIDs::ampSustain);
    synthParams.ampRelease = getF(ParameterIDs::ampRelease);
    synthParams.ampVelSens = getF(ParameterIDs::ampVelSens);
    
    // Filter Envelope
    synthParams.filtAttack = getF(ParameterIDs::filtAttack);
    synthParams.filtDecay = getF(ParameterIDs::filtDecay);
    synthParams.filtSustain = getF(ParameterIDs::filtSustain);
    synthParams.filtRelease = getF(ParameterIDs::filtRelease);
    synthParams.filtAmount = getF(ParameterIDs::filtAmount);
    synthParams.filtVelSens = getF(ParameterIDs::filtVelSens);
    
    // LFO 1
    synthParams.lfo1Wave = static_cast<int>(getI(ParameterIDs::lfo1Wave));
    synthParams.lfo1Rate = getF(ParameterIDs::lfo1Rate);
    synthParams.lfo1Amount = getF(ParameterIDs::lfo1Amount);
    synthParams.lfo1Delay = getF(ParameterIDs::lfo1Delay);
    synthParams.lfo1Fade = getF(ParameterIDs::lfo1Fade);
    
    // LFO 2
    synthParams.lfo2Wave = static_cast<int>(getI(ParameterIDs::lfo2Wave));
    synthParams.lfo2Rate = getF(ParameterIDs::lfo2Rate);
    synthParams.lfo2Amount = getF(ParameterIDs::lfo2Amount);
    synthParams.lfo2Delay = getF(ParameterIDs::lfo2Delay);
    synthParams.lfo2Fade = getF(ParameterIDs::lfo2Fade);
    
    // Apply to synth
    synth.setParams(synthParams);
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

//==============================================================================
juce::AudioProcessorEditor* AnalogSynthAudioProcessor::createEditor()
{
    return new AnalogSynthAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AnalogSynthAudioProcessor();
}