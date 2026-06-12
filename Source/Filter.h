#pragma once

// JUCE module includes
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

class Filter
{
public:
    enum Type { LP24, LP12, HP24, HP12, BP24, BP12 };
    
    void prepare(double sampleRate);
    void setType(Type t) { type = t; }
    void setCutoff(float hz) { cutoff = hz; updateCoefficients(); }
    void setResonance(float r) { resonance = juce::jlimit(0.0f, 0.99f, r); updateCoefficients(); }
    void setDrive(float d) { drive = d; }
    void setKeyTrack(float kt) { keyTrack = kt; }
    void setEnvelopeAmount(float amt) { envAmount = amt; }
    void setEnvelopeValue(float val) { envValue = val; updateCoefficients(); }
    
    float process(float input);
    void reset();
    
private:
    void updateCoefficients();
    
    double sampleRate = 44100.0;
    Type type = LP24;
    float cutoff = 1000.0f;
    float resonance = 0.0f;
    float drive = 0.0f;
    float keyTrack = 0.5f;
    float envAmount = 0.0f;
    float envValue = 0.0f;
    
    // 4-stage cascade for 24dB
    struct Stage { float state = 0.0f; float g = 0.0f; };
    std::array<Stage, 4> stages;
    float filterG = 0.0f;
    float filterK = 0.0f;
};