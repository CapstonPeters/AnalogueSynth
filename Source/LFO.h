#pragma once

// JUCE module includes
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

class LFO
{
public:
    enum Waveform { Sine, Triangle, Saw, Square, SampleHold, Random };
    enum SyncMode { Free, Sync1_4, Sync1_8, Sync1_16, Sync1_32 };
    
    void prepare(double sampleRate);
    void setWaveform(Waveform w) { waveform = w; }
    void setRate(float hz) { rate = hz; updateIncrement(); }
    void setSync(SyncMode s) { syncMode = s; updateIncrement(); }
    void setFadeIn(float t) { fadeInTime = t; }
    void setSampleRate(double sr) { sampleRate = sr; updateIncrement(); }
    void setTempo(double bpm) { tempo = bpm; updateIncrement(); }
    
    float process();  // Returns -1 to 1
    void reset() { phase = 0.0; fadeInCounter = 0.0f; lastRandom = 0.0f; }
    void noteOn() { phase = 0.0; fadeInCounter = 0.0f; }
    
private:
    void updateIncrement();
    float generateWaveform(float phase);
    
    double sampleRate = 44100.0;
    double tempo = 120.0;
    float rate = 1.0f;
    float fadeInTime = 0.0f;
    float fadeInCounter = 0.0f;
    Waveform waveform = Sine;
    SyncMode syncMode = Free;
    double phase = 0.0;
    double phaseIncrement = 0.0;
    float lastRandom = 0.0f;
    juce::Random random;
};