#pragma once

// JUCE module includes
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

class Envelope
{
public:
    enum Stage { Idle, Attack, Decay, Sustain, Release };
    
    void prepare(double sampleRate);
    void noteOn();
    void noteOff();
    void setAttack(float a) { attack = a; updateRates(); }
    void setDecay(float d) { decay = d; updateRates(); }
    void setSustain(float s) { sustain = s; }
    void setRelease(float r) { release = r; updateRates(); }
    void setCurve(float c) { curve = c; }  // -1 to 1, 0 = linear
    
    float process();  // Returns 0-1 envelope value
    bool isActive() const { return stage != Idle; }
    void reset() { stage = Idle; value = 0.0f; }
    
private:
    void updateRates();
    float calculateRate(float time);
    float applyCurve(float linear);
    
    double sampleRate = 44100.0;
    Stage stage = Idle;
    float value = 0.0f;
    float attack = 0.01f, decay = 0.5f, sustain = 0.7f, release = 0.5f;
    float curve = 0.0f;
    float attackRate = 0.0f, decayRate = 0.0f, releaseRate = 0.0f;
};