#include "Oscillator.h"

void Oscillator::prepare(double sr)
{
    sampleRate = sr;
    updateIncrement();
}

void Oscillator::updateIncrement()
{
    float detuneRatio = std::pow(2.0f, detune / 1200.0f);
    phaseIncrement = (frequency * detuneRatio) / sampleRate;
}

float Oscillator::process()
{
    float out = nextSampleBLEP((float)phase);
    phase += phaseIncrement;
    if (phase >= 1.0) phase -= 1.0;
    return out * level;
}

float Oscillator::nextSampleBLEP(float t)
{
    switch (waveform) {
        case Sine:
            return std::sin(2.0f * juce::MathConstants<float>::pi * t);
            
        case Triangle: {
            float tri = 2.0f * std::abs(2.0f * t - 1.0f) - 1.0f;
            // BLEP for triangle
            float dt = (float)phaseIncrement;
            if (t < dt) return tri - polyBLEP(t);
            else if (t > 1.0f - dt) return tri + polyBLEP(1.0f - t);
            return tri;
        }
            
        case Saw: {
            float saw = 2.0f * t - 1.0f;
            // BLEP for saw
            float dt = (float)phaseIncrement;
            if (t < dt) return saw - polyBLEP(t);
            else if (t > 1.0f - dt) return saw + polyBLEP(1.0f - t);
            return saw;
        }
            
        case Square: {
            float sq = (t < pulseWidth) ? 1.0f : -1.0f;
            // BLEP for square (two transitions)
            float dt = (float)phaseIncrement;
            if (t < dt) return sq - polyBLEP(t);
            else if (t > 1.0f - dt) return sq + polyBLEP(1.0f - t);
            float pwPos = pulseWidth;
            if (std::abs(t - pwPos) < dt) {
                if (t < pwPos) return sq + polyBLEP(t - pwPos);
                else return sq - polyBLEP(t - pwPos);
            }
            return sq;
        }
    }
    return 0.0f;
}

float Oscillator::polyBLEP(float t)
{
    float dt = (float)phaseIncrement;
    if (dt <= 0) return 0.0f;
    t /= dt;
    if (t >= 1.0f) return 0.0f;
    return t + t - t * t - 0.5f;
}