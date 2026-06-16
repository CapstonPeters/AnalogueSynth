#include "PluginEditor.h"
#include "PluginProcessor.h"

AnalogSynthAudioProcessorEditor::AnalogSynthAudioProcessorEditor (AnalogSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), testToneButton ("Test Tone"), waveTypeComboBox()
{
    setSize (600, 400);
    setResizable (true, true);
    
    addAndMakeVisible(testToneButton);
    testToneButton.setButtonText("Test Tone (A4)");
    testToneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2ECC71));
    testToneButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFE74C3C));
    testToneButton.setClickingTogglesState(true);
    testToneButton.onClick = [this]()
    {
        processorRef.setTestToneActive(testToneButton.getToggleState());
        testToneButton.setButtonText(testToneButton.getToggleState() ? "Stop Tone" : "Test Tone (A4)");
        testToneButton.setColour(juce::TextButton::buttonColourId, testToneButton.getToggleState() ? juce::Colour(0xFFE74C3C) : juce::Colour(0xFF2ECC71));
        repaint();
    };

    addAndMakeVisible(waveTypeComboBox);
    waveTypeComboBox.addItem("Sine", 1);
    waveTypeComboBox.addItem("Square", 2);
    waveTypeComboBox.addItem("Saw", 3);
    waveTypeComboBox.setSelectedId(1);
    waveTypeComboBox.setTooltip("Wave type (UI only - not connected to audio yet)");
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
    g.drawText ("Click 'Test Tone' to verify audio output, or play MIDI", getLocalBounds().removeFromBottom(80), juce::Justification::centred);
}

void AnalogSynthAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto buttonArea = bounds.removeFromTop(60).removeFromBottom(40);
    testToneButton.setBounds(buttonArea.reduced(200, 0));

    auto comboArea = bounds.removeFromTop(40).removeFromBottom(30);
    waveTypeComboBox.setBounds(comboArea.reduced(200, 0));
}