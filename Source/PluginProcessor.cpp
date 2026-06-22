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
        oscillators[i].setWavetableIndex(p.osc[i].wavetableIndex);
        oscillators[i].setScan(p.osc[i].scan);
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
    
    // Mod Matrix
    modMatrix = p.modMatrix;
    filtAmount = p.filtAmount;
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
    float mw = modWheelVal;
    float at = aftertouchVal;
    float pb = pitchBendVal;
    float expr = exprPedalVal;

    // Pitch bend: shift oscillator frequencies
    float pbSemitones = pb * pitchBendRange;
    float pbFactor = std::pow(2.0f, pbSemitones / 12.0f);

    // Mod wheel → filter cutoff boost (0 to +24 semitones)
    float mwBoost = mw * 24.0f;
    
    // Filter cutoff modulation
    float filtMod = modMatrix.getModulation(ModDest::FilterCutoff, lfo1Out, lfo2Out, ampEnvOut, filtEnvOut, velocity, keyTrack, mw, at, pb, expr);
    filtMod += filtEnvOut * filtAmount;
    filtMod += mwBoost; // Mod wheel opens filter
    filter.setCutoffMod(filtMod);
    
    // Oscillator pitch modulation (LFO1 -> pitch)
    float pitchMod = modMatrix.getModulation(ModDest::Osc1Pitch, lfo1Out, lfo2Out, ampEnvOut, filtEnvOut, velocity, keyTrack, mw, at, pb, expr);
    float pitchMult = std::pow(2.0f, pitchMod / 12.0f) * pbFactor;
    
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
    
    // Apply amp envelope with velocity scaling
    float velScale = 0.3f + velocity * 0.7f; // velocity 0→0.3, velocity 1→1.0
    oscSumL *= ampEnvOut * velScale;
    oscSumR *= ampEnvOut * velScale;
    
    // Process filter
    float filteredL, filteredR;
    filter.processBlock(&oscSumL, &oscSumR, &filteredL, &filteredR, 1);
    
    outL = filteredL;
    outR = filteredR;

    // Expression pedal → output level
    outL *= expr;
    outR *= expr;
    
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
    
    // === Arpeggiator ===
    bool arpOn = apvts.getRawParameterValue(ParameterIDs::arpEnabled)->load() > 0.0f;
    if (arpOn)
    {
        // Collect held notes from MIDI + track play order
        for (const auto metadata : midiMessages)
        {
            auto msg = metadata.getMessage();
            if (msg.isNoteOn())
            {
                int n = msg.getNoteNumber();
                if (std::find(arpNotes.begin(), arpNotes.end(), n) == arpNotes.end())
                {
                    arpNotes.push_back(n);
                    arpNoteOrder.push_back(n);  // track play order
                }
                std::sort(arpNotes.begin(), arpNotes.end());
            }
            else if (msg.isNoteOff())
            {
                int n = msg.getNoteNumber();
                auto itN = std::find(arpNotes.begin(), arpNotes.end(), n);
                if (itN != arpNotes.end()) arpNotes.erase(itN);
                auto itO = std::find(arpNoteOrder.begin(), arpNoteOrder.end(), n);
                if (itO != arpNoteOrder.end()) arpNoteOrder.erase(itO);
            }
        }
        midiMessages.clear();

        // Read arp params fresh from APVTS
        int    arpModeVal    = static_cast<int>(apvts.getRawParameterValue(ParameterIDs::arpMode)->load());
        int    arpRateIdx    = static_cast<int>(apvts.getRawParameterValue(ParameterIDs::arpRate)->load());
        int    arpOctavesVal = static_cast<int>(apvts.getRawParameterValue(ParameterIDs::arpOctaves)->load());
        float  arpGateVal    = apvts.getRawParameterValue(ParameterIDs::arpGate)->load();
        int    arpStepsVal   = static_cast<int>(apvts.getRawParameterValue(ParameterIDs::arpSteps)->load());
        float  arpSwingVal   = apvts.getRawParameterValue(ParameterIDs::arpSwing)->load();

        // Read per-step offsets and enables for Custom mode
        int    stepOffsets[16];
        bool   stepEnables[16];
        for (int i = 0; i < 16; ++i)
        {
            stepOffsets[i] = static_cast<int>(apvts.getRawParameterValue(ParameterIDs::arpStepOffset(i))->load());
            stepEnables[i] = apvts.getRawParameterValue(ParameterIDs::arpStepEnable(i))->load() > 0.0f;
        }

        // Determine rate in Hz (at 120 BPM reference)
        static const float rateTable[] = { 2.0f, 4.0f, 8.0f, 6.0f, 12.0f, 2.67f };
        int rateIdx = std::clamp(arpRateIdx, 0, 5);
        arpRateHz = rateTable[rateIdx];

        double sr = getSampleRate();
        if (sr <= 0) sr = 44100.0;

        double baseStepSamples = sr / arpRateHz;

        if (!arpNotes.empty())
        {
            int numNotes = static_cast<int>(arpNotes.size());
            int octaves  = std::clamp(arpOctavesVal, 1, 4);

            // Determine total steps and effective step count
            int totalSteps;
            bool isCustom = (arpModeVal == 5);
            bool isOrderPlayed = (arpModeVal == 4);

            if (isCustom)
                totalSteps = std::clamp(arpStepsVal, 2, 16);
            else
                totalSteps = numNotes * octaves;

            // Advance step counter with swing
            if (totalSteps > 0)
            {
                // Compute swing-adjusted step duration for this step
                double stepSamples = baseStepSamples;
                if (arpSwingVal > 0.001f && arpRateHz >= 4.0f)  // swing only at 1/8 and faster
                {
                    int swingPhase = arpStep % 2;
                    if (swingPhase == 0)
                        stepSamples = baseStepSamples * (1.0 - arpSwingVal * 0.5);
                    else
                        stepSamples = baseStepSamples * (1.0 + arpSwingVal * 0.5);
                }

                arpTimer += numSamples;
                while (arpTimer >= stepSamples && totalSteps > 0)
                {
                    arpTimer -= stepSamples;

                    // Note-off previous arp note
                    if (arpNoteOut >= 0)
                        synth.noteOff(arpNoteOut);

                    // Reset gate timer for new note
                    arpGateTimer = 0.0;

                    int noteNum = -1;

                    if (isCustom)
                    {
                        // Custom pattern: step through user-defined sequence
                        int stepIdx = arpStep % totalSteps;
                        // Find the next enabled step (skip rests/disbled)
                        int tries = 0;
                        while (tries < totalSteps && !stepEnables[stepIdx])
                        {
                            arpStep++;
                            stepIdx = arpStep % totalSteps;
                            tries++;
                        }
                        if (stepEnables[stepIdx] && numNotes > 0)
                        {
                            int baseNote = arpNotes[stepIdx % numNotes];
                            int offset = stepOffsets[stepIdx];
                            noteNum = baseNote + offset;
                        }
                        arpStep++;
                    }
                    else if (isOrderPlayed)
                    {
                        // Play notes in the order they were pressed
                        if (!arpNoteOrder.empty())
                        {
                            int idx = arpStep % static_cast<int>(arpNoteOrder.size());
                            int oct = (arpStep / static_cast<int>(arpNoteOrder.size())) % octaves;
                            noteNum = arpNoteOrder[idx] + oct * 12;
                        }
                        arpStep++;
                    }
                    else
                    {
                        // Standard modes (Up, Down, Up-Down, Random)
                        int baseIdx = arpStep % numNotes;
                        int oct = arpStep / numNotes;
                        if (oct >= octaves) { arpStep = 0; oct = 0; baseIdx = 0; }

                        noteNum = arpNotes[baseIdx] + oct * 12;

                        int mode = std::clamp(arpModeVal, 0, 3);
                        if (mode == 0) // Up
                        {
                            arpStep++;
                        }
                        else if (mode == 1) // Down
                        {
                            arpStep--;
                            if (arpStep < 0) arpStep = totalSteps - 1;
                        }
                        else if (mode == 2) // Up-Down
                        {
                            arpStep += arpDir;
                            if (arpStep >= totalSteps - 1) arpDir = -1;
                            else if (arpStep <= 0) arpDir = 1;
                        }
                        else // Random
                        {
                            int next;
                            do { next = rand() % totalSteps; }
                            while (totalSteps > 1 && next == arpLastRandom);
                            arpLastRandom = next;
                            arpStep = next;
                        }
                        arpStep = (arpStep % totalSteps + totalSteps) % totalSteps;
                    }

                    // Recompute swing for next step
                    if (arpSwingVal > 0.001f && arpRateHz >= 4.0f)
                    {
                        int nextSwingPhase = arpStep % 2;
                        if (nextSwingPhase == 0)
                            stepSamples = baseStepSamples * (1.0 - arpSwingVal * 0.5);
                        else
                            stepSamples = baseStepSamples * (1.0 + arpSwingVal * 0.5);
                    }

                    // Trigger note if valid
                    if (noteNum >= 0)
                    {
                        arpNoteOut = noteNum;
                        synth.noteOn(arpNoteOut, 0.8f);
                        arpNoteHeld = true;
                    }
                }

                // Gate handling: release note when gate time expires
                if (arpNoteHeld && arpNoteOut >= 0)
                {
                    double gateSamples = baseStepSamples * std::clamp(arpGateVal, 0.1f, 1.0f);
                    arpGateTimer += numSamples;
                    if (arpGateTimer >= gateSamples)
                    {
                        arpGateTimer -= gateSamples;
                        synth.noteOff(arpNoteOut);
                        arpNoteHeld = false;
                    }
                }
            }
        }
        else
        {
            // No notes held - stop arp
            if (arpNoteOut >= 0)
            {
                synth.noteOff(arpNoteOut);
                arpNoteOut = -1;
            }
            arpStep = 0;
            arpTimer = 0;
            arpNoteHeld = false;
            arpGateTimer = 0.0;
            arpLastRandom = -1;
            arpNoteOrder.clear();
        }
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

    auto addBool = [&](const char* id, const char* name, bool def)
    {
        params.push_back(std::make_unique<juce::AudioParameterBool>(id, name, def));
    };
    
    // Global
    addFloat(ParameterIDs::masterGain, "Master Gain", 0.0f, 1.0f, 0.5f, "dB");
    addInt(ParameterIDs::polyphony, "Polyphony", 1, 16, 8);
    addFloat(ParameterIDs::pitchBendRange, "Pitch Bend Range", 0.0f, 24.0f, 2.0f, "semitones");
    
    // Oscillator choices
    juce::StringArray oscWaves = {"Sine", "Triangle", "Saw", "Square", "Noise", "Wavetable"};
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
    addChoice(ParameterIDs::osc1WavetableIndex, "Osc 1 Wavetable", {"Sine", "Triangle", "Saw", "Square", "Moog Saw", "PWM Sweep", "Brass", "Soft Square", "FM Bell", "Vocal", "Additive 1", "Organ", "Pluck", "Chip", "Noise WT"}, 0);
    addFloat(ParameterIDs::osc1Scan, "Osc 1 Scan", 0.0f, 1.0f, 0.0f);
    
    // Osc 2
    addChoice(ParameterIDs::osc2Wave, "Osc 2 Wave", oscWaves, 2);
    addFloat(ParameterIDs::osc2Level, "Osc 2 Level", 0.0f, 1.0f, 0.5f);
    addFloat(ParameterIDs::osc2Pitch, "Osc 2 Pitch", -24.0f, 24.0f, 0.0f, "semitones");
    addFloat(ParameterIDs::osc2FineTune, "Osc 2 Fine Tune", -50.0f, 50.0f, 0.0f, "cents");
    addFloat(ParameterIDs::osc2Pan, "Osc 2 Pan", -1.0f, 1.0f, 0.0f);
    addInt(ParameterIDs::osc2Unison, "Osc 2 Unison", 1, 8, 1);
    addFloat(ParameterIDs::osc2Detune, "Osc 2 Detune", 0.0f, 50.0f, 0.0f, "cents");
    addFloat(ParameterIDs::osc2PulseWidth, "Osc 2 Pulse Width", 0.01f, 0.99f, 0.5f);
    addChoice(ParameterIDs::osc2WavetableIndex, "Osc 2 Wavetable", {"Sine", "Triangle", "Saw", "Square", "Moog Saw", "PWM Sweep", "Brass", "Soft Square", "FM Bell", "Vocal", "Additive 1", "Organ", "Pluck", "Chip", "Noise WT"}, 0);
    addFloat(ParameterIDs::osc2Scan, "Osc 2 Scan", 0.0f, 1.0f, 0.0f);
    
    // Osc 3
    addChoice(ParameterIDs::osc3Wave, "Osc 3 Wave", oscWaves, 2);
    addFloat(ParameterIDs::osc3Level, "Osc 3 Level", 0.0f, 1.0f, 0.3f);
    addFloat(ParameterIDs::osc3Pitch, "Osc 3 Pitch", -24.0f, 24.0f, 0.0f, "semitones");
    addFloat(ParameterIDs::osc3FineTune, "Osc 3 Fine Tune", -50.0f, 50.0f, 0.0f, "cents");
    addFloat(ParameterIDs::osc3Pan, "Osc 3 Pan", -1.0f, 1.0f, 0.0f);
    addInt(ParameterIDs::osc3Unison, "Osc 3 Unison", 1, 8, 1);
    addFloat(ParameterIDs::osc3Detune, "Osc 3 Detune", 0.0f, 50.0f, 0.0f, "cents");
    addFloat(ParameterIDs::osc3PulseWidth, "Osc 3 Pulse Width", 0.01f, 0.99f, 0.5f);
    addChoice(ParameterIDs::osc3WavetableIndex, "Osc 3 Wavetable", {"Sine", "Triangle", "Saw", "Square", "Moog Saw", "PWM Sweep", "Brass", "Soft Square", "FM Bell", "Vocal", "Additive 1", "Organ", "Pluck", "Chip", "Noise WT"}, 0);
    addFloat(ParameterIDs::osc3Scan, "Osc 3 Scan", 0.0f, 1.0f, 0.0f);
    
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
    
    // LFO 3
    addChoice(ParameterIDs::lfo3Wave, "LFO 3 Wave", lfoWaves, 1);
    addFloat(ParameterIDs::lfo3Rate, "LFO 3 Rate", 0.01f, 20.0f, 5.0f, "Hz");
    addFloat(ParameterIDs::lfo3Amount, "LFO 3 Amount", 0.0f, 1.0f, 0.0f);
    addFloat(ParameterIDs::lfo3Delay, "LFO 3 Delay", 0.0f, 5.0f, 0.0f, "s");
    addFloat(ParameterIDs::lfo3Fade, "LFO 3 Fade", 0.0f, 5.0f, 0.0f, "s");
    
    // LFO Sync
    addBool(ParameterIDs::lfo1Sync, "LFO 1 Sync", false);
    addBool(ParameterIDs::lfo2Sync, "LFO 2 Sync", false);
    addBool(ParameterIDs::lfo3Sync, "LFO 3 Sync", false);
    
    // LFO Note values (when synced)
    juce::StringArray lfoNotes = {"1/16", "1/8", "1/4", "1/2", "1 bar", "2 bars", "1/16T", "1/8T", "1/4T", "1/16D", "1/8D", "1/4D"};
    addChoice(ParameterIDs::lfo1Note, "LFO 1 Note", lfoNotes, 3);
    addChoice(ParameterIDs::lfo2Note, "LFO 2 Note", lfoNotes, 3);
    addChoice(ParameterIDs::lfo3Note, "LFO 3 Note", lfoNotes, 3);
    
    // LFO Phase
    addFloat(ParameterIDs::lfo1Phase, "LFO 1 Phase", 0.0f, 360.0f, 0.0f, "°");
    addFloat(ParameterIDs::lfo2Phase, "LFO 2 Phase", 0.0f, 360.0f, 0.0f, "°");
    addFloat(ParameterIDs::lfo3Phase, "LFO 3 Phase", 0.0f, 360.0f, 0.0f, "°");
    
    // LFO Trigger modes
    juce::StringArray lfoTrig = {"Free", "Retrig", "Single"};
    addChoice(ParameterIDs::lfo1Trigger, "LFO 1 Trigger", lfoTrig, 0);
    addChoice(ParameterIDs::lfo2Trigger, "LFO 2 Trigger", lfoTrig, 0);
    addChoice(ParameterIDs::lfo3Trigger, "LFO 3 Trigger", lfoTrig, 0);
    
    // LFO One-shot
    addBool(ParameterIDs::lfo1OneShot, "LFO 1 One-Shot", false);
    addBool(ParameterIDs::lfo2OneShot, "LFO 2 One-Shot", false);
    addBool(ParameterIDs::lfo3OneShot, "LFO 3 One-Shot", false);
    
    // Mod Matrix (8 slots - simplified as individual params for now)
    // In a full implementation, these would be structured differently

    // Arpeggiator
    addBool(ParameterIDs::arpEnabled, "Arp Enabled", false);
    addChoice(ParameterIDs::arpMode, "Arp Mode", {"Up", "Down", "Up-Down", "Random", "Order Played", "Custom"}, 0);
    addChoice(ParameterIDs::arpRate, "Arp Rate", {"1/4", "1/8", "1/16", "1/8T", "1/16T", "1/8D"}, 1);
    addInt(ParameterIDs::arpOctaves, "Arp Octaves", 1, 4, 1);
    addFloat(ParameterIDs::arpGate, "Arp Gate", 0.1f, 1.0f, 0.8f);
    addInt(ParameterIDs::arpSteps, "Arp Steps", 2, 16, 8);
    addFloat(ParameterIDs::arpSwing, "Arp Swing", 0.0f, 1.0f, 0.0f);
    // 16 per-step pitch offsets (-24 to +24 semitones)
    for (int i = 0; i < 16; ++i)
        addInt(ParameterIDs::arpStepOffset(i), ("Arp Step " + juce::String(i+1) + " Offset").toRawUTF8(), -24, 24, 0);
    // 16 per-step enables
    for (int i = 0; i < 16; ++i)
        addBool(ParameterIDs::arpStepEnable(i), ("Arp Step " + juce::String(i+1) + " Enable").toRawUTF8(), i < 8);
    // For now we'll skip detailed mod matrix params and use defaults
    
    // FX
    addBool(ParameterIDs::chorusOn, "Chorus On", false);
    addFloat(ParameterIDs::chorusRate, "Chorus Rate", 0.1f, 5.0f, 1.0f, "Hz");
    addFloat(ParameterIDs::chorusDepth, "Chorus Depth", 0.0f, 1.0f, 0.5f);
    addFloat(ParameterIDs::chorusMix, "Chorus Mix", 0.0f, 1.0f, 0.3f);
    addBool(ParameterIDs::flangerOn, "Flanger On", false);
    addFloat(ParameterIDs::flangerRate, "Flanger Rate", 0.05f, 5.0f, 0.5f, "Hz");
    addFloat(ParameterIDs::flangerDepth, "Flanger Depth", 0.0f, 1.0f, 0.5f);
    addFloat(ParameterIDs::flangerFeedback, "Flanger Feedback", 0.0f, 1.0f, 0.5f);
    addFloat(ParameterIDs::flangerMix, "Flanger Mix", 0.0f, 1.0f, 0.5f);
    addBool(ParameterIDs::phaserOn, "Phaser On", false);
    addFloat(ParameterIDs::phaserRate, "Phaser Rate", 0.05f, 5.0f, 0.3f, "Hz");
    addFloat(ParameterIDs::phaserDepth, "Phaser Depth", 0.0f, 1.0f, 0.5f);
    addFloat(ParameterIDs::phaserFeedback, "Phaser Feedback", 0.0f, 1.0f, 0.5f);
    addFloat(ParameterIDs::phaserMix, "Phaser Mix", 0.0f, 1.0f, 0.5f);
    addBool(ParameterIDs::delayOn, "Delay On", false);
    addFloat(ParameterIDs::delayTimeL, "Delay Time L", 0.02f, 2.0f, 0.5f, "s");
    addFloat(ParameterIDs::delayTimeR, "Delay Time R", 0.02f, 2.0f, 0.375f, "s");
    addFloat(ParameterIDs::delayFeedback, "Delay Feedback", 0.0f, 1.0f, 0.4f);
    addFloat(ParameterIDs::delayWet, "Delay Wet", 0.0f, 1.0f, 0.3f);
    addBool(ParameterIDs::reverbOn, "Reverb On", false);
    addFloat(ParameterIDs::reverbSize, "Reverb Size", 0.1f, 1.0f, 0.5f);
    addFloat(ParameterIDs::reverbDamp, "Reverb Damp", 0.0f, 1.0f, 0.5f);
    addFloat(ParameterIDs::reverbWet, "Reverb Wet", 0.0f, 1.0f, 0.2f);
    
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
        const char* wt[] = {ParameterIDs::osc1WavetableIndex, ParameterIDs::osc2WavetableIndex, ParameterIDs::osc3WavetableIndex};
        const char* scan[] = {ParameterIDs::osc1Scan, ParameterIDs::osc2Scan, ParameterIDs::osc3Scan};
        
        synthParams.osc[i].wave = static_cast<int>(getI(prefix[i]));
        synthParams.osc[i].level = getF(level[i]);
        synthParams.osc[i].pitch = getF(pitch[i]);
        synthParams.osc[i].fineTune = getF(fine[i]);
        synthParams.osc[i].pan = getF(pan[i]);
        synthParams.osc[i].unison = static_cast<int>(getI(unison[i]));
        synthParams.osc[i].detune = getF(detune[i]);
        synthParams.osc[i].pulseWidth = getF(pw[i]);
        synthParams.osc[i].wavetableIndex = static_cast<int>(getI(wt[i]));
        synthParams.osc[i].scan = getF(scan[i]);
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
    

    // Arpeggiator
    synthParams.arpEnabled = getI(ParameterIDs::arpEnabled) > 0;
    synthParams.arpMode    = static_cast<int>(getI(ParameterIDs::arpMode));
    synthParams.arpRate    = getF(ParameterIDs::arpRate);
    synthParams.arpOctaves = static_cast<int>(getI(ParameterIDs::arpOctaves));
    synthParams.arpGate    = getF(ParameterIDs::arpGate);
    synthParams.arpSteps   = static_cast<int>(getI(ParameterIDs::arpSteps));
    synthParams.arpSwing   = getF(ParameterIDs::arpSwing);
    for (int i = 0; i < 16; ++i)
    {
        synthParams.arpStepOffsets[i] = static_cast<int>(getI(ParameterIDs::arpStepOffset(i)));
        synthParams.arpStepEnables[i] = getI(ParameterIDs::arpStepEnable(i)) > 0;
    }
    
    // FX
    synthParams.chorusOn   = getI(ParameterIDs::chorusOn) > 0;
    synthParams.chorusRate  = getF(ParameterIDs::chorusRate);
    synthParams.chorusDepth = getF(ParameterIDs::chorusDepth);
    synthParams.chorusMix   = getF(ParameterIDs::chorusMix);
    synthParams.flangerOn   = getI(ParameterIDs::flangerOn) > 0;
    synthParams.flangerRate  = getF(ParameterIDs::flangerRate);
    synthParams.flangerDepth = getF(ParameterIDs::flangerDepth);
    synthParams.flangerFeedback = getF(ParameterIDs::flangerFeedback);
    synthParams.flangerMix   = getF(ParameterIDs::flangerMix);
    synthParams.phaserOn    = getI(ParameterIDs::phaserOn) > 0;
    synthParams.phaserRate   = getF(ParameterIDs::phaserRate);
    synthParams.phaserDepth  = getF(ParameterIDs::phaserDepth);
    synthParams.phaserFeedback = getF(ParameterIDs::phaserFeedback);
    synthParams.phaserMix    = getF(ParameterIDs::phaserMix);
    synthParams.delayOn     = getI(ParameterIDs::delayOn) > 0;
    synthParams.delayTimeL   = getF(ParameterIDs::delayTimeL);
    synthParams.delayTimeR   = getF(ParameterIDs::delayTimeR);
    synthParams.delayFeedback = getF(ParameterIDs::delayFeedback);
    synthParams.delayWet     = getF(ParameterIDs::delayWet);
    synthParams.reverbOn    = getI(ParameterIDs::reverbOn) > 0;
    synthParams.reverbSize   = getF(ParameterIDs::reverbSize);
    synthParams.reverbDamp   = getF(ParameterIDs::reverbDamp);
    synthParams.reverbWet    = getF(ParameterIDs::reverbWet);
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