#include "ModMatrix.h"
#include "LFO.h"
#include "Envelope.h"

ModMatrix::ModMatrix(juce::AudioProcessorValueTreeState& apvts)
{
    updateParameters(apvts);
}

void ModMatrix::prepare(double sr)
{
    sampleRate = sr;
}

void ModMatrix::updateParameters(juce::AudioProcessorValueTreeState& apvts)
{
    for (int i = 0; i < NumSlots; ++i) {
        slots[i].source = (int)*apvts.getRawParameterValue(ParameterID::modSrc + std::to_string(i));
        slots[i].dest = (int)*apvts.getRawParameterValue(ParameterID::modDest + std::to_string(i));
        slots[i].amount = *apvts.getRawParameterValue(ParameterID::modAmount + std::to_string(i));
    }
}

float ModMatrix::getModulatedValue(int destIndex, float baseValue)
{
    float result = baseValue;
    for (int i = 0; i < NumSlots; ++i) {
        if (slots[i].dest == destIndex && slots[i].source != Off) {
            float srcVal = sourceValues[slots[i].source];
            result += slots[i].amount * srcVal;
        }
    }
    return result;
}

void ModMatrix::updateSourceValues(const LFO& lfo1, const LFO& lfo2, 
                                   const Envelope& fenv, const Envelope& aenv,
                                   float velocity, float modWheel, float aftertouch, float pitchBend)
{
    sourceValues[LFO1] = lfo1.process();
    sourceValues[LFO2] = lfo2.process();
    sourceValues[FilterEnv] = fenv.process();
    sourceValues[AmpEnv] = aenv.process();
    sourceValues[Velocity] = velocity * 2.0f - 1.0f;  // 0-1 to -1 to 1
    sourceValues[ModWheel] = modWheel * 2.0f - 1.0f;
    sourceValues[Aftertouch] = aftertouch * 2.0f - 1.0f;
    sourceValues[PitchBend] = pitchBend;
}