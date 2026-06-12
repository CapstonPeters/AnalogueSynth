#include "Filter.h"

void Filter::prepare(double sr)
{
    sampleRate = sr;
    updateCoefficients();
}

void Filter::reset()
{
    for (auto& s : stages) s.state = 0.0f;
}

void Filter::updateCoefficients()
{
    // Calculate effective cutoff with key track and envelope
    float effectiveCutoff = cutoff;
    
    // Apply envelope modulation
    if (envAmount != 0.0f) {
        float envMod = envAmount * envValue; // -1 to 1 * 0 to 1
        effectiveCutoff *= std::pow(2.0f, envMod * 4.0f); // +/- 4 octaves
    }
    
    // Clamp
    effectiveCutoff = juce::jlimit(20.0f, 20000.0f, effectiveCutoff);
    
    // Bilinear transform for 1-pole sections
    float wc = 2.0f * std::tan(juce::MathConstants<float>::pi * effectiveCutoff / sampleRate);
    float g = wc / (1.0f + wc);
    
    // Resonance feedback
    float k = 2.0f * resonance;
    
    for (auto& s : stages) {
        s.g = g;
    }
    
    // Store for process
    filterG = g;
    filterK = k;
}

float Filter::process(float input)
{
    // Apply drive (soft saturation before filter)
    if (drive > 0.0f) {
        input = std::tanh(input * (1.0f + drive * 10.0f)) / (1.0f + drive * 10.0f);
    }
    
    float x = input;
    
    // 4-stage cascade (24dB)
    for (int i = 0; i < 4; ++i) {
        // Resonance feedback from last stage output
        float fb = (i == 0 && type == LP24) ? stages[3].state * filterK : 0.0f;
        
        // One-pole lowpass
        float y = stages[i].state + filterG * (x - stages[i].state + fb);
        stages[i].state = y;
        x = y;
    }
    
    // Handle different filter types by tapping different stages
    switch (type) {
        case LP24: return stages[3].state;
        case LP12: return stages[1].state;
        case HP24: return input - stages[3].state;
        case HP12: return input - stages[1].state;
        case BP24: return stages[1].state - stages[3].state;
        case BP12: return stages[0].state - stages[2].state;
    }
    return stages[3].state;
}