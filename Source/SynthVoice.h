#pragma once

// JUCE module includes
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

#include "Oscillator.h"
#include "Filter.h"
#include "Envelope.h"
#include "LFO.h"
#include "ModMatrix.h"

class SynthVoice  : public juce::SynthesiserVoice
{
public:
    SynthVoice(ModMatrix* mm, int unisonVoices = 1, float unisonDetune = 10.0f);
    ~SynthVoice() override = default;
    
    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int newPitchWheelValue) override;
    void controllerMoved (int controllerNumber, int newControllerValue) override;
    void prepare (double sampleRate, int samplesPerBlock);
    void release();
    void updateParameters(juce::AudioProcessorValueTreeState& apvts);
    void renderNextBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    
private:
    // 3 Oscillators + sub + noise
    std::array<Oscillator, 3> oscillators;
    Oscillator subOsc;
    juce::Random random;
    
    // Filter
    Filter filter;
    
    // Envelopes
    Envelope filterEnv;
    Envelope ampEnv;
    
    // LFOs
    LFO lfo1;
    LFO lfo2;
    
    // Mod Matrix
    ModMatrix* modMatrix;
    
    // State
    double sampleRate = 44100.0;
    int unisonVoices = 1;
    float unisonDetune = 10.0f;
    float currentPitchWheel = 0.0f;
    float currentModWheel = 0.0f;
    float currentExpression = 0.0f;
    float currentAftertouch = 0.0f;
    bool sustainPedalDown = false;
    float lastVelocity = 0.5f;
    
    // Per-unison voice state
    struct UnisonVoice {
        float detuneOffset = 0.0f;
        std::array<Oscillator, 3> oscillators;
        Filter filter;
        Envelope filterEnv;
        Envelope ampEnv;
    };
    std::vector<UnisonVoice> unisonVoicesVec;
    
    void updateUnisonParameters();
    float getModulatedValue(int paramIndex, float baseValue);
    float generateNoise();
};