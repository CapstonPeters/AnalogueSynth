#pragma once

// JUCE module includes
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

class ModMatrix
{
public:
    struct Slot {
        int source = 0;      // Index into modSources
        int dest = 0;        // Index into modDests
        float amount = 0.0f; // -1 to 1
    };
    
    static constexpr int NumSlots = 8;
    
    ModMatrix() = default;
    explicit ModMatrix(juce::AudioProcessorValueTreeState& apvts);
    
    void prepare(double sampleRate);
    void updateParameters(juce::AudioProcessorValueTreeState& apvts);
    float getModulatedValue(int destIndex, float baseValue);
    void updateSourceValues(const LFO& lfo1, const LFO& lfo2, 
                            const Envelope& fenv, const Envelope& aenv,
                            float velocity, float modWheel, float aftertouch, float pitchBend);
    
    // Sources (must match parameter order)
    enum Source { Off, LFO1, LFO2, FilterEnv, AmpEnv, Velocity, ModWheel, Aftertouch, PitchBend, NumSources };
    enum Dest { 
        Off_dest, Osc1Pitch, Osc2Pitch, Osc3Pitch, AllOscPitch,
        Osc1Level, Osc2Level, Osc3Level,
        Osc1PW, Osc2PW, Osc3PW,
        FilterCutoff, FilterRes, FilterDrive,
        AmpLevel, Pan, NumDests 
    };
    
private:
    double sampleRate = 44100.0;
    std::array<Slot, NumSlots> slots;
    std::array<float, NumSources> sourceValues;
};