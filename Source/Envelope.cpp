#include "Envelope.h"

void Envelope::prepare(double sr)
{
    sampleRate = sr;
    updateRates();
}

void Envelope::updateRates()
{
    attackRate = calculateRate(attack);
    decayRate = calculateRate(decay);
    releaseRate = calculateRate(release);
}

float Envelope::calculateRate(float time)
{
    if (time <= 0.0f) return 1.0f;
    return 1.0f - std::exp(-1.0f / (time * sampleRate));
}

void Envelope::noteOn()
{
    stage = Attack;
    value = 0.0f;
}

void Envelope::noteOff()
{
    if (stage != Idle) stage = Release;
}

float Envelope::applyCurve(float linear)
{
    if (curve == 0.0f) return linear;
    if (curve > 0.0f) {
        // Exponential (convex)
        return std::pow(linear, 1.0f + curve * 4.0f);
    } else {
        // Logarithmic (concave)
        return 1.0f - std::pow(1.0f - linear, 1.0f - curve * 4.0f);
    }
}

float Envelope::process()
{
    switch (stage) {
        case Idle:
            return 0.0f;
            
        case Attack:
            value += (1.0f - value) * attackRate;
            value = applyCurve(value);
            if (value >= 0.999f) {
                value = 1.0f;
                stage = Decay;
            }
            break;
            
        case Decay:
            value += (sustain - value) * decayRate;
            if (value <= sustain + 0.001f) {
                value = sustain;
                stage = Sustain;
            }
            break;
            
        case Sustain:
            value = sustain;
            break;
            
        case Release:
            value *= (1.0f - releaseRate);
            value = applyCurve(value);
            if (value <= 0.001f) {
                value = 0.0f;
                stage = Idle;
            }
            break;
    }
    return value;
}