#pragma once

#ifndef JucePlugin_Name
#define JucePlugin_Name "AnalogSynth"
#endif

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <cmath>
#include <atomic>
#include <array>

//==============================================================================
// Parameter IDs (string constants for APVTS)
struct ParamID
{
    // Global
    static constexpr const char* masterVolume = "masterVolume";
    static constexpr const char* masterTune   = "masterTune";
    static constexpr const char* polyphony    = "polyphony";
    static constexpr const char* unisonVoices = "unisonVoices";
    static constexpr const char* unisonDetune = "unisonDetune";
    static constexpr const char* unisonSpread = "unisonSpread";

    // Oscillator 1
    static constexpr const char* osc1Waveform   = "osc1Waveform";
    static constexpr const char* osc1Level      = "osc1Level";
    static constexpr const char* osc1Pitch      = "osc1Pitch";
    static constexpr const char* osc1Detune     = "osc1Detune";
    static constexpr const char* osc1Pan        = "osc1Pan";
    static constexpr const char* osc1PulseWidth = "osc1PulseWidth";

    // Oscillator 2
    static constexpr const char* osc2Waveform   = "osc2Waveform";
    static constexpr const char* osc2Level      = "osc2Level";
    static constexpr const char* osc2Pitch      = "osc2Pitch";
    static constexpr const char* osc2Detune     = "osc2Detune";
    static constexpr const char* osc2Pan        = "osc2Pan";
    static constexpr const char* osc2PulseWidth = "osc2PulseWidth";

    // Oscillator 3
    static constexpr const char* osc3Waveform   = "osc3Waveform";
    static constexpr const char* osc3Level      = "osc3Level";
    static constexpr const char* osc3Pitch      = "osc3Pitch";
    static constexpr const char* osc3Detune     = "osc3Detune";
    static constexpr const char* osc3Pan        = "osc3Pan";
    static constexpr const char* osc3PulseWidth = "osc3PulseWidth";

    // Oscillator Mix
    static constexpr const char* oscMixBalance12 = "oscMixBalance12";
    static constexpr const char* oscMixBalance123 = "oscMixBalance123";
    static constexpr const char* noiseLevel      = "noiseLevel";
    static constexpr const char* subLevel        = "subLevel";

    // Filter
    static constexpr const char* filterType      = "filterType";
    static constexpr const char* filterCutoff    = "filterCutoff";
    static constexpr const char* filterResonance = "filterResonance";
    static constexpr const char* filterDrive     = "filterDrive";
    static constexpr const char* filterKeyTrack  = "filterKeyTrack";
    static constexpr const char* filterVelTrack  = "filterVelTrack";
    static constexpr const char* filterEnvAmount = "filterEnvAmount";

    // Amp Envelope
    static constexpr const char* ampAttack  = "ampAttack";
    static constexpr const char* ampDecay   = "ampDecay";
    static constexpr const char* ampSustain = "ampSustain";
    static constexpr const char* ampRelease = "ampRelease";

    // Filter Envelope
    static constexpr const char* filtAttack  = "filtAttack";
    static constexpr const char* filtDecay   = "filtDecay";
    static constexpr const char* filtSustain = "filtSustain";
    static constexpr const char* filtRelease = "filtRelease";

    // LFO 1
    static constexpr const char* lfo1Rate    = "lfo1Rate";
    static constexpr const char* lfo1Waveform = "lfo1Waveform";
    static constexpr const char* lfo1Delay   = "lfo1Delay";
    static constexpr const char* lfo1Fade    = "lfo1Fade";

    // LFO 2
    static constexpr const char* lfo2Rate    = "lfo2Rate";
    static constexpr const char* lfo2Waveform = "lfo2Waveform";
    static constexpr const char* lfo2Delay   = "lfo2Delay";
    static constexpr const char* lfo2Fade    = "lfo2Fade";

    // Mod Matrix (8 slots)
    static constexpr const char* modSrc[8]  = { "mod1Src", "mod2Src", "mod3Src", "mod4Src", "mod5Src", "mod6Src", "mod7Src", "mod8Src" };
    static constexpr const char* modDst[8]  = { "mod1Dst", "mod2Dst", "mod3Dst", "mod4Dst", "mod5Dst", "mod6Dst", "mod7Dst", "mod8Dst" };
    static constexpr const char* modAmt[8]  = { "mod1Amt", "mod2Amt", "mod3Amt", "mod4Amt", "mod5Amt", "mod6Amt", "mod7Amt", "mod8Amt" };
};

//==============================================================================
// Enumerations
enum class Waveform : int { Sine = 0, Triangle = 1, Saw = 2, Square = 3, Noise = 4 };
enum class FilterType : int { LP12 = 0, LP24 = 1, HP12 = 2, HP24 = 3, BP12 = 4, BP24 = 5, Notch = 6 };
enum class ModSource : int { None = 0, LFO1 = 1, LFO2 = 2, AmpEnv = 3, FiltEnv = 4, Velocity = 5, ModWheel = 6, Aftertouch = 7, PitchBend = 8, KeyTrack = 9 };
enum class ModDest : int { None = 0, Osc1Pitch = 1, Osc2Pitch = 2, Osc3Pitch = 3, AllOscPitch = 4, Osc1Level = 5, Osc2Level = 6, Osc3Level = 7, FilterCutoff = 8, FilterResonance = 9, FilterDrive = 10, AmpLevel = 11, Pan = 12, LFO1Rate = 13, LFO2Rate = 14 };

//==============================================================================
// Oscillator
class Oscillator
{
public:
    void prepare(double sampleRate) { sr = sampleRate; }
    void setWaveform(Waveform w) { waveform = w; }
    void setFrequency(float f) { baseFreq = f; updatePhaseInc(); }
    void setDetune(float cents) { detune = cents; updatePhaseInc(); }
    void setPulseWidth(float pw) { pulseWidth = juce::jlimit(0.01f, 0.99f, pw); }
    void setLevel(float l) { level = l; }
    void setPan(float p) { pan = p; }
    void noteOn() { phase = 0; for (auto& p : unisonPhases) p = 0; active = true; }
    void noteOff() { active = false; }
    bool isActive() const { return active; }

    void setUnison(int voices, float detuneCents, float spread)
    {
        unisonVoices = juce::jlimit(1, 8, voices);
        unisonDetune = detuneCents;
        unisonSpread = spread;
        unisonPhases.resize(unisonVoices);
    }

    float process()
    {
        if (!active) return 0.0f;

        float sum = 0.0f;
        for (int v = 0; v < unisonVoices; ++v)
        {
            float ph = unisonPhases[v];
            float sample = 0.0f;

            switch (waveform)
            {
                case Waveform::Sine:
                    sample = std::sin(ph);
                    break;
                case Waveform::Triangle:
                    sample = 2.0f * std::abs(ph / juce::MathConstants<float>::pi - std::floor(ph / juce::MathConstants<float>::pi + 0.5f)) - 1.0f;
                    break;
                case Waveform::Saw:
                    sample = 2.0f * (ph / (2.0f * juce::MathConstants<float>::pi) - std::floor(0.5f + ph / (2.0f * juce::MathConstants<float>::pi)));
                    break;
                case Waveform::Square:
                    sample = (ph < pulseWidth * 2.0f * juce::MathConstants<float>::pi) ? 1.0f : -1.0f;
                    break;
                case Waveform::Noise:
                    sample = 2.0f * (static_cast<float>(std::rand()) / RAND_MAX) - 1.0f;
                    break;
            }

            sum += sample;
            unisonPhases[v] += unisonPhaseIncs[v];
            if (unisonPhases[v] > 2.0f * juce::MathConstants<float>::pi) unisonPhases[v] -= 2.0f * juce::MathConstants<float>::pi;
        }

        return (sum / unisonVoices) * level;
    }

    float getPhase() const { return phase; }

private:
    void updatePhaseInc()
    {
        float freq = baseFreq * std::pow(2.0f, detune / 1200.0f);
        phaseInc = freq / sr * 2.0f * juce::MathConstants<float>::pi;

        unisonPhaseIncs.resize(unisonVoices);
        for (int v = 0; v < unisonVoices; ++v)
        {
            float spreadAmt = (unisonVoices > 1) ? (static_cast<float>(v) - (unisonVoices - 1) * 0.5f) * unisonDetune / 100.0f : 0.0f;
            float vFreq = freq * std::pow(2.0f, spreadAmt / 1200.0f);
            unisonPhaseIncs[v] = vFreq / sr * 2.0f * juce::MathConstants<float>::pi;
        }
    }

    double sr = 44100;
    Waveform waveform = Waveform::Saw;
    float baseFreq = 440;
    float detune = 0;
    float pulseWidth = 0.5f;
    float level = 1.0f;
    float pan = 0.0f;
    float phaseInc = 0;
    float phase = 0;
    bool active = false;

    int unisonVoices = 1;
    float unisonDetune = 0;
    float unisonSpread = 0;
    std::vector<float> unisonPhases;
    std::vector<float> unisonPhaseIncs;
};

//==============================================================================
// ADSR Envelope
class EnvelopeADSR
{
public:
    enum class Stage { Idle, Attack, Decay, Sustain, Release };

    void prepare(double sampleRate) { sr = sampleRate; }
    void setParams(float a, float d, float s, float r)
    {
        attack  = juce::jmax(0.001f, a);
        decay   = juce::jmax(0.001f, d);
        sustain = juce::jlimit(0.0f, 1.0f, s);
        release = juce::jmax(0.001f, r);
    }
    void noteOn() { stage = Stage::Attack; level = 0.0f; }
    void noteOff() { if (stage != Stage::Idle) stage = Stage::Release; }
    bool isActive() const { return stage != Stage::Idle; }
    float process()
    {
        switch (stage)
        {
            case Stage::Attack:
                level += 1.0f / (attack * sr);
                if (level >= 1.0f) { level = 1.0f; stage = Stage::Decay; }
                break;
            case Stage::Decay:
                level -= (1.0f - sustain) / (decay * sr);
                if (level <= sustain) { level = sustain; stage = Stage::Sustain; }
                break;
            case Stage::Sustain:
                break;
            case Stage::Release:
                level -= sustain / (release * sr);
                if (level <= 0.0f) { level = 0.0f; stage = Stage::Idle; }
                break;
            case Stage::Idle:
                break;
        }
        return level;
    }
    float getLevel() const { return level; }
    Stage getStage() const { return stage; }

private:
    double sr = 44100;
    Stage stage = Stage::Idle;
    float level = 0.0f;
    float attack = 0.01f, decay = 0.1f, sustain = 0.7f, release = 0.2f;
};

//==============================================================================
// LFO
class LFO
{
public:
    enum class Waveform { Sine = 0, Triangle = 1, Saw = 2, Square = 3, SampleHold = 4 };

    void prepare(double sampleRate) { sr = sampleRate; }
    void setWaveform(Waveform w) { waveform = w; }
    void setRate(float hz) { rate = hz; updatePhaseInc(); }
    void setDelay(float d) { delay = d; delaySamples = static_cast<int>(d * sr); }
    void setFade(float f) { fade = f; fadeSamples = static_cast<int>(f * sr); }
    void noteOn() { phase = 0; delayCounter = 0; fadeCounter = 0; active = true; }
    void noteOff() { active = false; }
    bool isActive() const { return active; }
    float process()
    {
        if (!active) return 0.0f;

        // Delay
        if (delayCounter < delaySamples) { ++delayCounter; return 0.0f; }

        // Fade in
        float gain = 1.0f;
        if (fadeCounter < fadeSamples) { gain = static_cast<float>(fadeCounter) / fadeSamples; ++fadeCounter; }

        float out = 0.0f;
        switch (waveform)
        {
            case Waveform::Sine: out = std::sin(phase); break;
            case Waveform::Triangle: out = 2.0f * std::abs(phase / juce::MathConstants<float>::pi - std::floor(phase / juce::MathConstants<float>::pi + 0.5f)) - 1.0f; break;
            case Waveform::Saw: out = 2.0f * (phase / (2.0f * juce::MathConstants<float>::pi) - std::floor(0.5f + phase / (2.0f * juce::MathConstants<float>::pi))); break;
            case Waveform::Square: out = (phase < juce::MathConstants<float>::pi) ? 1.0f : -1.0f; break;
            case Waveform::SampleHold: if (++shCounter >= shRate) { shCounter = 0; shValue = 2.0f * (static_cast<float>(std::rand()) / RAND_MAX) - 1.0f; } out = shValue; break;
        }

        phase += phaseInc;
        if (phase > 2.0f * juce::MathConstants<float>::pi) phase -= 2.0f * juce::MathConstants<float>::pi;

        return out * gain;
    }

private:
    void updatePhaseInc() { phaseInc = rate / sr * 2.0f * juce::MathConstants<float>::pi; shRate = static_cast<int>(sr / rate); }

    double sr = 44100;
    Waveform waveform = Waveform::Sine;
    float rate = 1.0f;
    float delay = 0.0f;
    float fade = 0.0f;
    float phaseInc = 0;
    float phase = 0;
    bool active = false;
    int delaySamples = 0, fadeSamples = 0, delayCounter = 0, fadeCounter = 0;
    int shCounter = 0, shRate = 44100;
    float shValue = 0;
};

//==============================================================================
// Filter (4-pole ladder style)
class Filter
{
public:
    void prepare(double sampleRate) { sr = sampleRate; }
    void setType(FilterType t) { type = t; }
    void setCutoff(float c) { cutoff = juce::jlimit(20.0f, 20000.0f, c); updateCoeffs(); }
    void setResonance(float r) { resonance = juce::jlimit(0.0f, 1.0f, r); updateCoeffs(); }
    void setDrive(float d) { drive = juce::jlimit(0.0f, 10.0f, d); }
    void setKeyTrack(float k) { keyTrack = k; }
    void noteOn() { for (auto& s : states) s = 0; }
    float process(float input)
    {
        float x = input * (1.0f + drive * 0.1f); // light saturation

        for (int i = 0; i < 4; ++i)
        {
            float g = coeff * (1.0f + resonance * 4.0f);
            float y = g * (x - states[i]) + states[i];
            states[i] = y + g * (y - states[i]); // 2-pole per stage = 4 total
            x = std::tanh(y);
        }
        return x;
    }
    float processStereo(float& left, float& right)
    {
        left  = process(left);
        right = process(right);
        return 0;
    }

private:
    void updateCoeffs()
    {
        float fc = cutoff / sr;
        fc = juce::jlimit(0.0f, 0.49f, fc);
        coeff = 2.0f * std::sin(juce::MathConstants<float>::pi * fc);
    }

    double sr = 44100;
    FilterType type = FilterType::LP24;
    float cutoff = 10000;
    float resonance = 0;
    float drive = 0;
    float keyTrack = 0;
    float coeff = 0;
    std::array<float, 4> states = {0, 0, 0, 0};
};

//==============================================================================
// Modulation Matrix (8 slots)
class ModMatrix
{
public:
    struct Slot
    {
        ModSource source = ModSource::None;
        ModDest dest = ModDest::None;
        float amount = 0.0f; // -1 to 1
        bool enabled = false;
    };

    void setSlot(int index, ModSource src, ModDest dst, float amt)
    {
        if (index >= 0 && index < 8)
        {
            slots[index].source = src;
            slots[index].dest = dst;
            slots[index].amount = juce::jlimit(-1.0f, 1.0f, amt);
            slots[index].enabled = (src != ModSource::None && dst != ModDest::None && amt != 0.0f);
        }
    }

    float getModValue(int index, float sourceValue) const
    {
        if (index >= 0 && index < 8 && slots[index].enabled)
            return sourceValue * slots[index].amount;
        return 0.0f;
    }

    const Slot& getSlot(int index) const { return slots[index]; }

private:
    std::array<Slot, 8> slots;
};

//==============================================================================
// Synth Voice
class SynthVoice : public juce::SynthesiserVoice
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        for (auto& o : oscillators) o.prepare(sampleRate);
        for (auto& o : subOscillators) o.prepare(sampleRate);
        noiseOsc.prepare(sampleRate);
        ampEnv.prepare(sampleRate);
        filtEnv.prepare(sampleRate);
        lfo1.prepare(sampleRate);
        lfo2.prepare(sampleRate);
        filter.prepare(sampleRate);
    }

    void setParams(const juce::AudioProcessorValueTreeState& apvts)
    {
        // Osc 1
        oscillators[0].setWaveform(static_cast<Waveform>(apvts.getRawParameterValue(ParamID::osc1Waveform)->load()));
        oscillators[0].setLevel(apvts.getRawParameterValue(ParamID::osc1Level)->load());
        oscillators[0].setPan(apvts.getRawParameterValue(ParamID::osc1Pan)->load());
        oscillators[0].setPulseWidth(apvts.getRawParameterValue(ParamID::osc1PulseWidth)->load());

        // Osc 2
        oscillators[1].setWaveform(static_cast<Waveform>(apvts.getRawParameterValue(ParamID::osc2Waveform)->load()));
        oscillators[1].setLevel(apvts.getRawParameterValue(ParamID::osc2Level)->load());
        oscillators[1].setPan(apvts.getRawParameterValue(ParamID::osc2Pan)->load());
        oscillators[1].setPulseWidth(apvts.getRawParameterValue(ParamID::osc2PulseWidth)->load());

        // Osc 3
        oscillators[2].setWaveform(static_cast<Waveform>(apvts.getRawParameterValue(ParamID::osc3Waveform)->load()));
        oscillators[2].setLevel(apvts.getRawParameterValue(ParamID::osc3Level)->load());
        oscillators[2].setPan(apvts.getRawParameterValue(ParamID::osc3Pan)->load());
        oscillators[2].setPulseWidth(apvts.getRawParameterValue(ParamID::osc3PulseWidth)->load());

        // Mix
        noiseLevel = apvts.getRawParameterValue(ParamID::noiseLevel)->load();
        subLevel   = apvts.getRawParameterValue(ParamID::subLevel)->load();

        // Filter
        filter.setType(static_cast<FilterType>(apvts.getRawParameterValue(ParamID::filterType)->load()));
        filter.setCutoff(apvts.getRawParameterValue(ParamID::filterCutoff)->load());
        filter.setResonance(apvts.getRawParameterValue(ParamID::filterResonance)->load());
        filter.setDrive(apvts.getRawParameterValue(ParamID::filterDrive)->load());
        filterKeyTrack = apvts.getRawParameterValue(ParamID::filterKeyTrack)->load();
        filterVelTrack = apvts.getRawParameterValue(ParamID::filterVelTrack)->load();
        filterEnvAmount = apvts.getRawParameterValue(ParamID::filterEnvAmount)->load();

        // Envelopes
        ampEnv.setParams(
            apvts.getRawParameterValue(ParamID::ampAttack)->load(),
            apvts.getRawParameterValue(ParamID::ampDecay)->load(),
            apvts.getRawParameterValue(ParamID::ampSustain)->load(),
            apvts.getRawParameterValue(ParamID::ampRelease)->load()
        );
        filtEnv.setParams(
            apvts.getRawParameterValue(ParamID::filtAttack)->load(),
            apvts.getRawParameterValue(ParamID::filtDecay)->load(),
            apvts.getRawParameterValue(ParamID::filtSustain)->load(),
            apvts.getRawParameterValue(ParamID::filtRelease)->load()
        );

        // LFOs
        lfo1.setWaveform(static_cast<LFO::Waveform>(apvts.getRawParameterValue(ParamID::lfo1Waveform)->load()));
        lfo1.setRate(apvts.getRawParameterValue(ParamID::lfo1Rate)->load());
        lfo1.setDelay(apvts.getRawParameterValue(ParamID::lfo1Delay)->load());
        lfo1.setFade(apvts.getRawParameterValue(ParamID::lfo1Fade)->load());

        lfo2.setWaveform(static_cast<LFO::Waveform>(apvts.getRawParameterValue(ParamID::lfo2Waveform)->load()));
        lfo2.setRate(apvts.getRawParameterValue(ParamID::lfo2Rate)->load());
        lfo2.setDelay(apvts.getRawParameterValue(ParamID::lfo2Delay)->load());
        lfo2.setFade(apvts.getRawParameterValue(ParamID::lfo2Fade)->load());

        // Mod Matrix
        for (int i = 0; i < 8; ++i)
        {
            int src = static_cast<int>(apvts.getRawParameterValue(ParamID::modSrc[i])->load());
            int dst = static_cast<int>(apvts.getRawParameterValue(ParamID::modDst[i])->load());
            float amt = apvts.getRawParameterValue(ParamID::modAmt[i])->load();
            modMatrix.setSlot(i, static_cast<ModSource>(src), static_cast<ModDest>(dst), amt);
        }
    }

    bool canPlaySound(juce::SynthesiserSound*) override { return true; }

    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override
    {
        note = midiNoteNumber;
        noteVel = velocity;
        float freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);

        // Oscillators
        for (int i = 0; i < 3; ++i)
        {
            oscillators[i].setFrequency(freq);
            oscillators[i].noteOn();
        }
        subOscillators[0].setFrequency(freq * 0.5f); // sub = -1 octave
        subOscillators[0].noteOn();
        noiseOsc.noteOn();

        ampEnv.noteOn();
        filtEnv.noteOn();
        lfo1.noteOn();
        lfo2.noteOn();
        filter.noteOn();

        // Update filter cutoff with key tracking
        float baseCutoff = filterCutoffBase;
        float keyTrackAmt = (midiNoteNumber - 60) * filterKeyTrack * 1200.0f; // cents
        float velTrackAmt = (velocity - 0.5f) * filterVelTrack * 2400.0f;
        filter.setCutoff(baseCutoff * std::pow(2.0f, (keyTrackAmt + velTrackAmt) / 1200.0f));
    }

    void stopNote(float velocity, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            ampEnv.noteOff();
            filtEnv.noteOff();
        }
        else
        {
            clearCurrentNote();
            for (auto& o : oscillators) o.noteOff();
            subOscillators[0].noteOff();
            noiseOsc.noteOff();
            lfo1.noteOff();
            lfo2.noteOff();
        }
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (!isVoiceActive()) return;

        for (int i = 0; i < numSamples; ++i)
        {
            // Modulation sources
            float lfo1Val = lfo1.process();
            float lfo2Val = lfo2.process();
            float ampEnvVal = ampEnv.process();
            float filtEnvVal = filtEnv.process();

            // Apply mod matrix to filter cutoff
            float modFilterCutoff = 0;
            for (int m = 0; m < 8; ++m)
            {
                const auto& slot = modMatrix.getSlot(m);
                if (!slot.enabled) continue;
                float srcVal = getModSourceValue(slot.source, lfo1Val, lfo2Val, ampEnvVal, filtEnvVal, noteVel);
                modFilterCutoff += modMatrix.getModValue(m, srcVal) * filterEnvAmount * 2400.0f; // cents
            }

            if (modFilterCutoff != 0)
            {
                float newCutoff = filterCutoffBase * std::pow(2.0f, modFilterCutoff / 1200.0f);
                filter.setCutoff(newCutoff);
            }

            // Generate oscillator signals
            float oscSum = 0;
            for (int o = 0; o < 3; ++o)
                oscSum += oscillators[o].process();

            // Sub oscillator
            float sub = subOscillators[0].process() * subLevel;

            // Noise
            float noise = noiseOsc.process() * noiseLevel;

            float signal = (oscSum + sub + noise) * ampEnvVal;

            // Filter
            filter.processStereo(signal, signal);

            // Output
            for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
                outputBuffer.addSample(ch, startSample + i, signal);
        }

        if (!ampEnv.isActive() && !filtEnv.isActive())
            clearCurrentNote();
    }

private:
    float getModSourceValue(ModSource src, float lfo1, float lfo2, float ampEnv, float filtEnv, float vel) const
    {
        switch (src)
        {
            case ModSource::LFO1: return lfo1;
            case ModSource::LFO2: return lfo2;
            case ModSource::AmpEnv: return ampEnv;
            case ModSource::FiltEnv: return filtEnv;
            case ModSource::Velocity: return vel;
            case ModSource::KeyTrack: return (note - 60) / 60.0f;
            default: return 0;
        }
    }

    double sr = 44100;
    int note = 60;
    float noteVel = 1.0f;

    std::array<Oscillator, 3> oscillators;
    std::array<Oscillator, 1> subOscillators;
    Oscillator noiseOsc;

    EnvelopeADSR ampEnv;
    EnvelopeADSR filtEnv;
    LFO lfo1, lfo2;
    Filter filter;
    ModMatrix modMatrix;

    float noiseLevel = 0, subLevel = 0;
    float filterCutoffBase = 10000;
    float filterKeyTrack = 0, filterVelTrack = 0, filterEnvAmount = 0;
};

//==============================================================================
// Simple sound class for Synthesiser
class SimpleSynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel (int /*midiChannel*/) override { return true; }
};

//==============================================================================
// Synth (voice management)
class Synth : public juce::Synthesiser
{
public:
    void setParams(const juce::AudioProcessorValueTreeState& apvts)
    {
        for (auto* v : voices)
            if (auto* sv = dynamic_cast<SynthVoice*>(v))
                sv->setParams(apvts);
    }

    // Expose voices for iteration
    const juce::OwnedArray<juce::SynthesiserVoice>& getVoicesArray() const { return voices; }
    juce::OwnedArray<juce::SynthesiserVoice>& getVoicesArray() { return voices; }
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

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    Synth synth;
    TestOscillator testToneOsc;
    std::atomic<bool> testToneActive{false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogSynthAudioProcessor)
};