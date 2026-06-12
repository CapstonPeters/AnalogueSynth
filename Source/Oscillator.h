#pragma once

// JUCE module includes
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class Oscillator
{
public:
    enum Waveform { Sine, Triangle, Saw, Square };
    
    void prepare(double sampleRate);
    void setWaveform(Waveform w) { waveform = w; }
    void setFrequency(float hz) { frequency = hz; updateIncrement(); }
    void setDetune(float cents) { detune = cents; updateIncrement(); }
    void setPulseWidth(float pw) { pulseWidth = juce::jlimit(0.01f, 0.99f, pw); }
    void setLevel(float l) { level = l; }
    
    float process();
    void reset() { phase = 0.0; }
    
private:
    void updateIncrement();
    float nextSampleBLEP(float t);
    float polyBLEP(float t);
    
    double sampleRate = 44100.0;
    float frequency = 440.0f;
    float detune = 0.0f;
    float pulseWidth = 0.5f;
    float level = 1.0f;
    Waveform waveform = Saw;
    double phase = 0.0;
    double phaseIncrement = 0.0;
    float lastOutput = 0.0f;
};