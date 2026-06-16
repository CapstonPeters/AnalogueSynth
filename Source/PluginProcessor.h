#pragma once

#ifndef JucePlugin_Name
#define JucePlugin_Name "AnalogSynth"
#endif

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <array>
#include <atomic>
#include <cmath>
#include <algorithm>

//==============================================================================
// Parameter IDs
struct ParameterIDs
{
    // Global
    static constexpr auto masterGain = "masterGain";
    static constexpr auto polyphony = "polyphony";
    static constexpr auto pitchBendRange = "pitchBendRange";
    
    // Oscillators (3 main + sub + noise)
    static constexpr auto osc1Wave = "osc1Wave";
    static constexpr auto osc1Level = "osc1Level";
    static constexpr auto osc1Pitch = "osc1Pitch";
    static constexpr auto osc1FineTune = "osc1FineTune";
    static constexpr auto osc1Pan = "osc1Pan";
    static constexpr auto osc1Unison = "osc1Unison";
    static constexpr auto osc1Detune = "osc1Detune";
    static constexpr auto osc1PulseWidth = "osc1PulseWidth";
    
    static constexpr auto osc2Wave = "osc2Wave";
    static constexpr auto osc2Level = "osc2Level";
    static constexpr auto osc2Pitch = "osc2Pitch";
    static constexpr auto osc2FineTune = "osc2FineTune";
    static constexpr auto osc2Pan = "osc2Pan";
    static constexpr auto osc2Unison = "osc2Unison";
    static constexpr auto osc2Detune = "osc2Detune";
    static constexpr auto osc2PulseWidth = "osc2PulseWidth";
    
    static constexpr auto osc3Wave = "osc3Wave";
    static constexpr auto osc3Level = "osc3Level";
    static constexpr auto osc3Pitch = "osc3Pitch";
    static constexpr auto osc3FineTune = "osc3FineTune";
    static constexpr auto osc3Pan = "osc3Pan";
    static constexpr auto osc3Unison = "osc3Unison";
    static constexpr auto osc3Detune = "osc3Detune";
    static constexpr auto osc3PulseWidth = "osc3PulseWidth";
    
    static constexpr auto subWave = "subWave";
    static constexpr auto subLevel = "subLevel";
    static constexpr auto subPitch = "subPitch";
    
    static constexpr auto noiseType = "noiseType";
    static constexpr auto noiseLevel = "noiseLevel";
    
    // Filter
    static constexpr auto filterType = "filterType";
    static constexpr auto filterCutoff = "filterCutoff";
    static constexpr auto filterResonance = "filterResonance";
    static constexpr auto filterDrive = "filterDrive";
    static constexpr auto filterKeyTrack = "filterKeyTrack";
    static constexpr auto filterVelTrack = "filterVelTrack";
    
    // Amp Envelope
    static constexpr auto ampAttack = "ampAttack";
    static constexpr auto ampDecay = "ampDecay";
    static constexpr auto ampSustain = "ampSustain";
    static constexpr auto ampRelease = "ampRelease";
    static constexpr auto ampVelSens = "ampVelSens";
    
    // Filter Envelope
    static constexpr auto filtAttack = "filtAttack";
    static constexpr auto filtDecay = "filtDecay";
    static constexpr auto filtSustain = "filtSustain";
    static constexpr auto filtRelease = "filtRelease";
    static constexpr auto filtAmount = "filtAmount";
    static constexpr auto filtVelSens = "filtVelSens";
    
    // LFO 1
    static constexpr auto lfo1Wave = "lfo1Wave";
    static constexpr auto lfo1Rate = "lfo1Rate";
    static constexpr auto lfo1Amount = "lfo1Amount";
    static constexpr auto lfo1Delay = "lfo1Delay";
    static constexpr auto lfo1Fade = "lfo1Fade";
    
    // LFO 2
    static constexpr auto lfo2Wave = "lfo2Wave";
    static constexpr auto lfo2Rate = "lfo2Rate";
    static constexpr auto lfo2Amount = "lfo2Amount";
    static constexpr auto lfo2Delay = "lfo2Delay";
    static constexpr auto lfo2Fade = "lfo2Fade";
    
    // Mod Matrix (8 slots: source, dest, amount)
    static constexpr auto modMatrixSource = "modMatrixSource";
    static constexpr auto modMatrixDest = "modMatrixDest";
    static constexpr auto modMatrixAmount = "modMatrixAmount";
};

//==============================================================================
// FastRandom - xorshift32 for thread-safe noise
class FastRandom
{
public:
    explicit FastRandom(uint32_t seed = 1) : state(seed) {}
    
    inline float nextFloat()
    {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return state * 2.3283064e-10f; // 1/2^32
    }
    
    inline float nextFloatBipolar() { return nextFloat() * 2.0f - 1.0f; }
    
private:
    uint32_t state;
};

//==============================================================================
// Oscillator - supports multiple waveforms, unison, pulse width
class Oscillator
{
public:
    enum WaveType { Sine = 0, Triangle = 1, Saw = 2, Square = 3, Noise = 4 };
    
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        for (auto& osc : unisonOscs) osc.phase = 0;
        noiseGen = FastRandom(static_cast<uint32_t>(sampleRate * 1000));
    }
    
    void setFrequency(float baseFreq) { freq = baseFreq; updatePhaseInc(); }
    void setWaveType(WaveType wt) { waveType = wt; }
    void setLevel(float l) { level = l; }
    void setPan(float p) { pan = p; }
    void setUnison(int u) { unison = std::clamp(u, 1, 8); }
    void setDetune(float d) { detune = d; }
    void setPulseWidth(float pw) { pulseWidth = std::clamp(pw, 0.01f, 0.99f); }
    
    void noteOn() 
    { 
        for (auto& osc : unisonOscs) osc.phase = 0; 
        active = true; 
    }
    void noteOff() { active = false; }
    bool isActive() const { return active; }
    
    float process()
    {
        if (!active) return 0.0f;
        
        float sum = 0.0f;
        const int voices = unison;
        
        for (int v = 0; v < voices; ++v)
        {
            auto& osc = unisonOscs[v];
            float voiceFreq = freq * std::pow(2.0f, detune * (v - (voices - 1) * 0.5f) / 1200.0f);
            float phaseInc = voiceFreq / sr * 2.0 * juce::MathConstants<double>::pi;
            osc.phaseInc = phaseInc;
            
            float sample = 0.0f;
            switch (waveType)
            {
                case Sine:
                    sample = std::sin(osc.phase);
                    break;
                case Triangle:
                    sample = 2.0f * std::abs(osc.phase / juce::MathConstants<double>::pi - 1.0f) - 1.0f;
                    break;
                case Saw:
                    sample = 2.0f * (osc.phase / (2.0 * juce::MathConstants<double>::pi)) - 1.0f;
                    break;
                case Square:
                    sample = (osc.phase < pulseWidth * 2.0 * juce::MathConstants<double>::pi) ? 1.0f : -1.0f;
                    break;
                case Noise:
                    sample = noiseGen.nextFloatBipolar();
                    break;
            }
            
            sum += sample;
            osc.phase += phaseInc;
            if (osc.phase > 2.0 * juce::MathConstants<double>::pi) osc.phase -= 2.0 * juce::MathConstants<double>::pi;
        }
        
        return sum * level / voices;
    }
    
    float getLeftGain() const { return std::cos((pan + 1.0f) * juce::MathConstants<float>::pi / 4.0f); }
    float getRightGain() const { return std::sin((pan + 1.0f) * juce::MathConstants<float>::pi / 4.0f); }
    
private:
    struct UnisonOsc { double phase = 0; double phaseInc = 0; };
    
    double sr = 44100;
    float freq = 440;
    float level = 1.0f;
    float pan = 0.0f;
    int unison = 1;
    float detune = 0.0f;
    float pulseWidth = 0.5f;
    bool active = false;
    WaveType waveType = Saw;
    FastRandom noiseGen;
    std::array<UnisonOsc, 8> unisonOscs;
    
    void updatePhaseInc() {}
};

//==============================================================================
// ADSR Envelope
class ADSREnvelope
{
public:
    enum State { Idle, Attack, Decay, Sustain, Release };
    
    void prepare(double sampleRate) { sr = sampleRate; }
    
    void setParams(float attack, float decay, float sustain, float release)
    {
        attackTime = std::max(0.001f, attack);
        decayTime = std::max(0.001f, decay);
        sustainLevel = std::clamp(sustain, 0.0f, 1.0f);
        releaseTime = std::max(0.001f, release);
        updateCoeffs();
    }
    
    void noteOn(float velocity = 1.0f, float velSens = 0.5f)
    {
        level = 0.0f;
        targetLevel = 1.0f * (1.0f - velSens + velSens * velocity);
        state = Attack;
        samplesInState = 0;
    }
    
    void noteOff()
    {
        if (state != Idle && state != Release)
        {
            state = Release;
            samplesInState = 0;
        }
    }
    
    bool isActive() const { return state != Idle; }
    
    float process()
    {
        if (state == Idle) return 0.0f;
        
        float output = level;
        samplesInState++;
        
        switch (state)
        {
            case Attack:
                level += attackCoeff * (1.0f - level);
                if (level >= 0.999f || samplesInState >= attackSamples)
                {
                    level = 1.0f;
                    state = Decay;
                    samplesInState = 0;
                }
                break;
            case Decay:
                level += decayCoeff * (sustainLevel - level);
                if (std::abs(level - sustainLevel) < 0.001f || samplesInState >= decaySamples)
                {
                    level = sustainLevel;
                    state = (sustainLevel > 0.001f) ? Sustain : Release;
                    samplesInState = 0;
                }
                break;
            case Sustain:
                // Stay at sustain level until noteOff
                break;
            case Release:
                level += releaseCoeff * (0.0f - level);
                if (level <= 0.001f || samplesInState >= releaseSamples)
                {
                    level = 0.0f;
                    state = Idle;
                }
                break;
            default:
                break;
        }
        
        return output * targetLevel;
    }
    
    float getLevel() const { return level * targetLevel; }
    
private:
    void updateCoeffs()
    {
        attackSamples = static_cast<int>(attackTime * sr);
        decaySamples = static_cast<int>(decayTime * sr);
        releaseSamples = static_cast<int>(releaseTime * sr);
        
        attackCoeff = 1.0f - std::exp(-1.0f / std::max(1, attackSamples));
        decayCoeff = 1.0f - std::exp(-1.0f / std::max(1, decaySamples));
        releaseCoeff = 1.0f - std::exp(-1.0f / std::max(1, releaseSamples));
    }
    
    double sr = 44100;
    float attackTime = 0.01f, decayTime = 0.3f, sustainLevel = 0.7f, releaseTime = 0.3f;
    int attackSamples = 441, decaySamples = 13230, releaseSamples = 13230;
    float attackCoeff = 0.01f, decayCoeff = 0.0001f, releaseCoeff = 0.0001f;
    float targetLevel = 1.0f;
    float level = 0.0f;
    State state = Idle;
    int samplesInState = 0;
};

//==============================================================================
// LFO
class LFO
{
public:
    enum WaveType { Sine = 0, Triangle = 1, Saw = 2, Square = 3, SampleHold = 4 };
    
    void prepare(double sampleRate) { sr = sampleRate; }
    
    void setParams(WaveType wt, float rateHz, float amount, float delaySec, float fadeSec)
    {
        waveType = wt;
        rate = std::max(0.01f, rateHz);
        targetAmount = std::clamp(amount, 0.0f, 1.0f);
        delaySamples = static_cast<int>(delaySec * sr);
        fadeSamples = std::max(1, static_cast<int>(fadeSec * sr));
        updatePhaseInc();
    }
    
    void noteOn() 
    { 
        phase = 0; 
        delayCounter = 0; 
        fadeLevel = 0; 
        active = true; 
        shValue = 0; 
    }
    
    void noteOff() { active = false; }
    bool isActive() const { return active; }
    
    float process()
    {
        if (!active) return 0.0f;
        
        // Delay
        if (delayCounter < delaySamples)
        {
            delayCounter++;
            return 0.0f;
        }
        
        // Fade in
        if (fadeLevel < 1.0f)
        {
            fadeLevel += 1.0f / fadeSamples;
            fadeLevel = std::min(fadeLevel, 1.0f);
        }
        
        phase += phaseInc;
        if (phase >= 2.0 * juce::MathConstants<double>::pi) phase -= 2.0 * juce::MathConstants<double>::pi;
        
        float sample = 0.0f;
        switch (waveType)
        {
            case Sine:
                sample = std::sin(phase);
                break;
            case Triangle:
                sample = 2.0f * std::abs(phase / juce::MathConstants<double>::pi - 1.0f) - 1.0f;
                break;
            case Saw:
                sample = 2.0f * (phase / (2.0 * juce::MathConstants<double>::pi)) - 1.0f;
                break;
            case Square:
                sample = (phase < juce::MathConstants<double>::pi) ? 1.0f : -1.0f;
                break;
            case SampleHold:
                if (phase < phaseInc) // New cycle
                    shValue = (FastRandom{}).nextFloatBipolar();
                sample = shValue;
                break;
        }
        
        return sample * targetAmount * fadeLevel;
    }
    
private:
    void updatePhaseInc()
    {
        phaseInc = rate / sr * 2.0 * juce::MathConstants<double>::pi;
    }
    
    double sr = 44100;
    WaveType waveType = Sine;
    float rate = 1.0f;
    float targetAmount = 0.0f;
    int delaySamples = 0;
    int fadeSamples = 1;
    double phaseInc = 0;
    double phase = 0;
    int delayCounter = 0;
    float fadeLevel = 0;
    float shValue = 0;
    bool active = false;
};

//==============================================================================
// 4-Pole Ladder Filter with resonance
class LadderFilter
{
public:
    enum Type { LP4 = 0, LP2 = 1, HP4 = 2, HP2 = 3, BP = 4, Notch = 5 };
    
    void prepare(double sampleRate) { sr = sampleRate; }
    
    void setParams(Type t, float cutoffHz, float resonance, float drive, float keyTrack, float velTrack)
    {
        type = t;
        cutoff = std::clamp(cutoffHz, 20.0f, 20000.0f);
        res = std::clamp(resonance, 0.0f, 0.95f);
        this->drive = std::clamp(drive, 0.0f, 1.0f);
        this->keyTrack = keyTrack;
        this->velTrack = velTrack;
        updateCoeffs();
    }
    
    void setCutoffMod(float mod) // -1 to 1
    {
        float modulatedCutoff = cutoff * std::pow(2.0f, mod * 10.0f); // 10 octaves max
        modulatedCutoff = std::clamp(modulatedCutoff, 20.0f, 20000.0f);
        if (std::abs(modulatedCutoff - currentCutoff) > 1.0f)
        {
            currentCutoff = modulatedCutoff;
            updateCoeffs();
        }
    }
    
    void reset() { statesL.fill(0); statesR.fill(0); }
    
    inline void processBlock(const float* inL, const float* inR, float* outL, float* outR, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            float xL = inL[i] * (1.0f + drive * 0.5f);
            float xR = inR[i] * (1.0f + drive * 0.5f);
            
            // Soft clip input
            xL = std::tanh(xL);
            xR = std::tanh(xR);
            
            // Feedback
            const float fb = res * 4.0f * (statesL[3] + statesR[3]) * 0.5f;
            
            // 4 stages
            for (int s = 0; s < 4; ++s)
            {
                float in = (s == 0) ? xL - fb : statesL[s - 1];
                float out = statesL[s] + g * (in - statesL[s]);
                out = std::tanh(out); // Nonlinearity per stage
                statesL[s] = out;
            }
            
            for (int s = 0; s < 4; ++s)
            {
                float in = (s == 0) ? xR - fb : statesR[s - 1];
                float out = statesR[s] + g * (in - statesR[s]);
                out = std::tanh(out);
                statesR[s] = out;
            }
            
            float yL = statesL[3];
            float yR = statesR[3];
            
            switch (type)
            {
                case LP4: case LP2:
                    outL[i] = yL;
                    outR[i] = yR;
                    break;
                case HP4: case HP2:
                    outL[i] = xL - yL;
                    outR[i] = xR - yR;
                    break;
                case BP:
                    outL[i] = statesL[1] - statesL[3];
                    outR[i] = statesR[1] - statesR[3];
                    break;
                case Notch:
                    outL[i] = xL - (statesL[1] - statesL[3]);
                    outR[i] = xR - (statesR[1] - statesR[3]);
                    break;
            }
        }
    }
    
private:
    void updateCoeffs()
    {
        currentCutoff = cutoff;
        float wc = 2.0f * juce::MathConstants<float>::pi * cutoff / sr;
        g = wc / (1.0f + wc); // Simplified bilinear approx
        
        // Resonance compensation
        g = std::clamp(g, 0.001f, 0.999f);
    }
    
    double sr = 44100;
    Type type = LP4;
    float cutoff = 1000.0f;
    float currentCutoff = 1000.0f;
    float res = 0.0f;
    float drive = 0.0f;
    float keyTrack = 0.0f;
    float velTrack = 0.0f;
    float g = 0.5f;
    std::array<float, 4> statesL{};
    std::array<float, 4> statesR{};
};

//==============================================================================
// Modulation Source enum
enum class ModSource : uint8_t
{
    None = 0,
    LFO1 = 1,
    LFO2 = 2,
    AmpEnv = 3,
    FiltEnv = 4,
    Velocity = 5,
    Keytrack = 6,
    ModWheel = 7,
    Aftertouch = 8,
    PitchBend = 9,
    ExprPedal = 10
};

// Modulation Destination enum  
enum class ModDest : uint8_t
{
    None = 0,
    Osc1Pitch = 1,
    Osc2Pitch = 2,
    Osc3Pitch = 3,
    Osc1Level = 4,
    Osc2Level = 5,
    Osc3Level = 6,
    Osc1PW = 7,
    Osc2PW = 8,
    Osc3PW = 9,
    FilterCutoff = 10,
    FilterResonance = 11,
    FilterDrive = 12,
    AmpAttack = 13,
    AmpDecay = 14,
    AmpSustain = 15,
    AmpRelease = 16,
    FiltAttack = 17,
    FiltDecay = 18,
    FiltSustain = 19,
    FiltRelease = 20,
    FiltAmount = 21,
    LFO1Rate = 22,
    LFO2Rate = 23,
    LFO1Amount = 24,
    LFO2Amount = 25,
    MasterGain = 26,
    Pan = 27
};

// Mod Matrix Slot
struct ModSlot
{
    ModSource source = ModSource::None;
    ModDest dest = ModDest::None;
    float amount = 0.0f; // -1 to 1
    bool enabled = false;
};

//==============================================================================
// Mod Matrix Manager
class ModMatrix
{
public:
    static constexpr int NumSlots = 8;
    
    void setSlot(int index, ModSource src, ModDest dst, float amt, bool en)
    {
        if (index >= 0 && index < NumSlots)
        {
            slots[index].source = src;
            slots[index].dest = dst;
            slots[index].amount = std::clamp(amt, -1.0f, 1.0f);
            slots[index].enabled = en;
        }
    }
    
    float getModulation(ModDest dest, float lfo1Out, float lfo2Out, float ampEnvOut, float filtEnvOut, float velocity, float keyTrack, float modWheel, float aftertouch, float pitchBend, float exprPedal) const
    {
        float sum = 0.0f;
        for (const auto& slot : slots)
        {
            if (!slot.enabled || slot.dest != dest) continue;
            
            float srcVal = 0.0f;
            switch (slot.source)
            {
                case ModSource::LFO1: srcVal = lfo1Out; break;
                case ModSource::LFO2: srcVal = lfo2Out; break;
                case ModSource::AmpEnv: srcVal = ampEnvOut; break;
                case ModSource::FiltEnv: srcVal = filtEnvOut; break;
                case ModSource::Velocity: srcVal = velocity; break;
                case ModSource::Keytrack: srcVal = keyTrack; break;
                case ModSource::ModWheel: srcVal = modWheel; break;
                case ModSource::Aftertouch: srcVal = aftertouch; break;
                case ModSource::PitchBend: srcVal = pitchBend; break;
                case ModSource::ExprPedal: srcVal = exprPedal; break;
                default: break;
            }
            sum += srcVal * slot.amount;
        }
        return std::clamp(sum, -1.0f, 1.0f);
    }
    
    const ModSlot& getSlot(int index) const { 
        static ModSlot empty; 
        return (index >= 0 && index < NumSlots) ? slots[index] : empty; 
    }
    
private:
    std::array<ModSlot, NumSlots> slots;
};

//==============================================================================
// Synth Voice
class SynthVoice
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        for (auto& osc : oscillators) osc.prepare(sampleRate);
        subOsc.prepare(sampleRate);
        noiseOsc.prepare(sampleRate);
        ampEnv.prepare(sampleRate);
        filtEnv.prepare(sampleRate);
        lfo1.prepare(sampleRate);
        lfo2.prepare(sampleRate);
        filter.prepare(sampleRate);
    }
    
    void startNote(int midiNote, float velocity)
    {
        note = midiNote;
        this->velocity = velocity;
        float freq = juce::MidiMessage::getMidiNoteInHertz(midiNote);
        
        for (int i = 0; i < 3; ++i)
            oscillators[i].setFrequency(freq * std::pow(2.0f, oscPitchOffsets[i] / 12.0f));
        
        subOsc.setFrequency(freq * 0.5f); // One octave down
        
        for (auto& osc : oscillators) osc.noteOn();
        subOsc.noteOn();
        noiseOsc.noteOn();
        
        ampEnv.noteOn(velocity, ampVelSens);
        filtEnv.noteOn(velocity, filtVelSens);
        lfo1.noteOn();
        lfo2.noteOn();
        
        active = true;
        samplesSinceNoteOn = 0;
    }
    
    void stopNote()
    {
        for (auto& osc : oscillators) osc.noteOff();
        subOsc.noteOff();
        noiseOsc.noteOff();
        ampEnv.noteOff();
        filtEnv.noteOff();
        lfo1.noteOff();
        lfo2.noteOff();
    }
    
    bool isActive() const { return active; }
    int getNote() const { return note; }
    
    void updateParams(const class SynthParams& params);
    
    // Process one sample (stereo output)
    void process(float& outL, float& outR);
    
private:
    double sr = 44100;
    int note = -1;
    float velocity = 1.0f;
    bool active = false;
    int samplesSinceNoteOn = 0;
    
    // Oscillators
    std::array<Oscillator, 3> oscillators;
    Oscillator subOsc;
    Oscillator noiseOsc;
    float oscPitchOffsets[3] = {0, 0, 0};
    
    // Envelopes
    ADSREnvelope ampEnv;
    ADSREnvelope filtEnv;
    
    // LFOs
    LFO lfo1;
    LFO lfo2;
    
    // Filter
    LadderFilter filter;
    
    // Modulation values (per block)
    float lfo1Out = 0, lfo2Out = 0, ampEnvOut = 0, filtEnvOut = 0;
    float modWheel = 0, aftertouch = 0, pitchBend = 0, exprPedal = 1.0f;
    
    // Safety
    static constexpr int MaxVoiceSamples = 30 * 48000; // 30 seconds max
};

//==============================================================================
// All synth parameters (updated from APVTS)
struct SynthParams
{
    // Global
    float masterGain = 0.5f;
    int polyphony = 8;
    float pitchBendRange = 2.0f;
    
    // Oscillators
    struct OscParams
    {
        int wave = 2; // Saw
        float level = 0.7f;
        float pitch = 0.0f; // semitones
        float fineTune = 0.0f; // cents
        float pan = 0.0f;
        int unison = 1;
        float detune = 0.0f; // cents
        float pulseWidth = 0.5f;
    };
    OscParams osc[3];
    
    struct SubParams { int wave = 0; float level = 0.0f; float pitch = -12.0f; } sub;
    struct NoiseParams { int type = 0; float level = 0.0f; } noise;
    
    // Filter
    int filterType = 0; // LP4
    float filterCutoff = 1000.0f;
    float filterResonance = 0.0f;
    float filterDrive = 0.0f;
    float filterKeyTrack = 0.0f;
    float filterVelTrack = 0.0f;
    
    // Amp Envelope
    float ampAttack = 0.01f;
    float ampDecay = 0.3f;
    float ampSustain = 0.7f;
    float ampRelease = 0.3f;
    float ampVelSens = 0.5f;
    
    // Filter Envelope
    float filtAttack = 0.01f;
    float filtDecay = 0.3f;
    float filtSustain = 0.0f;
    float filtRelease = 0.3f;
    float filtAmount = 0.5f;
    float filtVelSens = 0.0f;
    
    // LFO 1
    int lfo1Wave = 0; // Sine
    float lfo1Rate = 1.0f;
    float lfo1Amount = 0.0f;
    float lfo1Delay = 0.0f;
    float lfo1Fade = 0.0f;
    
    // LFO 2
    int lfo2Wave = 1; // Triangle
    float lfo2Rate = 5.0f;
    float lfo2Amount = 0.0f;
    float lfo2Delay = 0.0f;
    float lfo2Fade = 0.0f;
    
    // Mod Matrix
    ModMatrix modMatrix;
};

//==============================================================================
// Synth Engine (voice manager)
class SynthEngine
{
public:
    void prepare(double sampleRate, int maxVoices)
    {
        sr = sampleRate;
        voices.clear();
        voices.resize(maxVoices);
        for (auto& v : voices) v.prepare(sampleRate);
        params.polyphony = maxVoices;
    }
    
    void setParams(const SynthParams& p) { params = p; updateVoiceParams(); }
    const SynthParams& getParams() const { return params; }
    
    void noteOn(int midiNote, float velocity)
    {
        // Voice stealing: find oldest active voice if at polyphony limit
        int oldestIdx = -1;
        int oldestSamples = -1;
        int freeIdx = -1;
        
        for (int i = 0; i < (int)voices.size(); ++i)
        {
            if (!voices[i].isActive())
            {
                freeIdx = i;
                break;
            }
            if (voices[i].samplesSinceNoteOn > oldestSamples)
            {
                oldestSamples = voices[i].samplesSinceNoteOn;
                oldestIdx = i;
            }
        }
        
        int idx = (freeIdx >= 0) ? freeIdx : oldestIdx;
        if (idx >= 0) voices[idx].startNote(midiNote, velocity);
    }
    
    void noteOff(int midiNote)
    {
        for (auto& v : voices)
            if (v.isActive() && v.getNote() == midiNote)
                v.stopNote();
    }
    
    void processBlock(float* outL, float* outR, int numSamples)
    {
        // Clear output
        std::fill(outL, outL + numSamples, 0.0f);
        std::fill(outR, outR + numSamples, 0.0f);
        
        // Process each voice
        for (auto& v : voices)
        {
            if (!v.isActive()) continue;
            
            float voiceL = 0, voiceR = 0;
            for (int i = 0; i < numSamples; ++i)
            {
                v.process(voiceL, voiceR);
                outL[i] += voiceL;
                outR[i] += voiceR;
            }
            v.samplesSinceNoteOn += numSamples;
        }
        
        // Apply master gain
        float g = params.masterGain;
        for (int i = 0; i < numSamples; ++i)
        {
            outL[i] *= g;
            outR[i] *= g;
        }
    }
    
    void setModWheel(float v) { modWheel = std::clamp(v, 0.0f, 1.0f); }
    void setAftertouch(float v) { aftertouch = std::clamp(v, 0.0f, 1.0f); }
    void setPitchBend(float v) { pitchBend = std::clamp(v, -1.0f, 1.0f); }
    void setExprPedal(float v) { exprPedal = std::clamp(v, 0.0f, 1.0f); }
    
private:
    void updateVoiceParams()
    {
        for (auto& v : voices) v.updateParams(params);
    }
    
    double sr = 44100;
    std::vector<SynthVoice> voices;
    SynthParams params;
    float modWheel = 0, aftertouch = 0, pitchBend = 0, exprPedal = 1.0f;
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

    // APVTS
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    const juce::AudioProcessorValueTreeState& getAPVTS() const { return apvts; }

    // Test tone control (for debugging audio path)
    void setTestToneActive(bool active) { testToneActive.store(active); if (active) testToneOsc.setActive(true); else testToneOsc.setActive(false); }
    bool isTestToneActive() const { return testToneActive.load(); }
    
    // Wave type control for test tone
    void setWaveType(int wt) { testToneWaveType = wt; testToneOsc.setWaveType(static_cast<Oscillator::WaveType>(wt)); }
    int getWaveType() const { return testToneWaveType; }

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    // Parameter layout
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    // Update synth params from APVTS
    void updateSynthParams();
    
    // Test tone oscillator (simple)
    class TestToneOsc
    {
    public:
        void prepare(double sr) { sampleRate = sr; }
        void setFrequency(float f) { freq = f; phaseInc = f / sampleRate * 2.0 * juce::MathConstants<double>::pi; }
        void setWaveType(Oscillator::WaveType wt) { waveType = wt; }
        void noteOn() { phase = 0; active = true; }
        void noteOff() { active = false; }
        void setActive(bool a) { active = a; if (a) phase = 0; }
        bool isActive() const { return active; }
        float process()
        {
            if (!active) return 0.0f;
            float sample = 0.0f;
            switch (waveType)
            {
                case Oscillator::Sine: sample = std::sin(phase); break;
                case Oscillator::Square: sample = (phase < juce::MathConstants<double>::pi) ? 1.0f : -1.0f; break;
                case Oscillator::Saw: sample = 2.0f * (phase / (2.0 * juce::MathConstants<double>::pi)) - 1.0f; break;
                default: sample = std::sin(phase); break;
            }
            sample *= 0.3f;
            phase += phaseInc;
            if (phase > 2.0 * juce::MathConstants<double>::pi) phase -= 2.0 * juce::MathConstants<double>::pi;
            return sample;
        }
    private:
        double sampleRate = 44100;
        double phase = 0, phaseInc = 0;
        float freq = 440;
        bool active = false;
        Oscillator::WaveType waveType = Oscillator::Sine;
    };
    
    TestToneOsc testToneOsc;
    std::atomic<bool> testToneActive{false};
    int testToneWaveType = 0;
    
    // Full synth
    SynthEngine synth;
    SynthParams synthParams;
    
    // APVTS
    juce::AudioProcessorValueTreeState apvts;
    
    // MIDI state
    float lastModWheel = 0, lastAftertouch = 0, lastPitchBend = 0, lastExprPedal = 1.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogSynthAudioProcessor)
};