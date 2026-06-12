#include "LFO.h"

void LFO::prepare(double sr)
{
    sampleRate = sr;
    updateIncrement();
}

void LFO::updateIncrement()
{
    float effectiveRate = rate;
    
    if (syncMode != Free && tempo > 0.0) {
        float noteValue = 1.0f;
        switch (syncMode) {
            case Sync1_4: noteValue = 4.0f; break;   // quarter note
            case Sync1_8: noteValue = 8.0f; break;   // eighth note
            case Sync1_16: noteValue = 16.0f; break; // sixteenth note
            case Sync1_32: noteValue = 32.0f; break; // thirty-second note
            default: break;
        }
        effectiveRate = (tempo / 60.0) * noteValue;
    }
    
    phaseIncrement = effectiveRate / sampleRate;
}

float LFO::process()
{
    // Handle fade-in
    float fadeIn = 1.0f;
    if (fadeInTime > 0.0f && fadeInCounter < fadeInTime) {
        fadeInCounter += 1.0f / sampleRate;
        fadeIn = juce::jlimit(0.0f, 1.0f, fadeInCounter / fadeInTime);
        fadeIn = fadeIn * fadeIn; // Quadratic fade-in
    }
    
    float out = generateWaveform((float)phase);
    phase += phaseIncrement;
    if (phase >= 1.0) phase -= 1.0;
    
    return out * fadeIn;
}

float LFO::generateWaveform(float phase)
{
    switch (waveform) {
        case Sine:
            return std::sin(2.0f * juce::MathConstants<float>::pi * phase);
            
        case Triangle:
            return 2.0f * std::abs(2.0f * phase - 1.0f) - 1.0f;
            
        case Saw:
            return 2.0f * phase - 1.0f;
            
        case Square:
            return (phase < 0.5f) ? 1.0f : -1.0f;
            
        case SampleHold: {
            // Update random value at phase wrap
            if (phase + phaseIncrement >= 1.0) {
                lastRandom = random.nextFloat() * 2.0f - 1.0f;
            }
            return lastRandom;
        }
            
        case Random: {
            // Smooth random (interpolated)
            if (phase + phaseIncrement >= 1.0) {
                float newRandom = random.nextFloat() * 2.0f - 1.0f;
                float t = (float)(phase + phaseIncrement - 1.0f) / (float)phaseIncrement;
                return juce::jmap(t, lastRandom, newRandom);
            }
            return lastRandom;
        }
    }
    return 0.0f;
}