/*
  ==============================================================================
  Plugin entry point for JUCE VST3
  ==============================================================================
*/

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AnalogSynthAudioProcessor();
}