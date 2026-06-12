#include "PluginProcessor.h"
#include "PluginEditor.h"

AnalogSynthAudioProcessorEditor::AnalogSynthAudioProcessorEditor (AnalogSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (600, 400);
    setResizable (true, true);
}

AnalogSynthAudioProcessorEditor::~AnalogSynthAudioProcessorEditor() = default;

void AnalogSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xFF1E1E2E));
    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawText ("AnalogSynth", getLocalBounds().removeFromTop(80), juce::Justification::centred);
    
    g.setFont (14.0f);
    g.setColour (juce::Colours::grey);
    g.drawText ("VST3 Analog Synthesizer - Minimal Build", getLocalBounds().removeFromBottom(100), juce::Justification::centred);
    g.drawText ("Click a key on your MIDI controller to hear a test tone", getLocalBounds().removeFromBottom(80), juce::Justification::centred);
}

void AnalogSynthAudioProcessorEditor::resized()
{
}

juce::AudioProcessorEditor* AnalogSynthAudioProcessor::createEditor()
{
    return new AnalogSynthAudioProcessorEditor (*this);
}