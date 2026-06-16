#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class AnalogSynthAudioProcessor;

class AnalogSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AnalogSynthAudioProcessorEditor (AnalogSynthAudioProcessor&);
    ~AnalogSynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AnalogSynthAudioProcessor& processorRef;
    juce::TextButton testToneButton;
    juce::ComboBox waveTypeComboBox;

    // Oscillators
    juce::ComboBox osc1WaveCombo, osc2WaveCombo, osc3WaveCombo, subWaveCombo, noiseWaveCombo;
    juce::Label osc1Label, osc2Label, osc3Label, subLabel, noiseLabel;

    // Filter
    juce::ComboBox filterTypeCombo;
    juce::Slider cutoffSlider, resonanceSlider, driveSlider;
    juce::Label filterLabel, cutoffLabel, resonanceLabel, driveLabel;

    // Amp Envelope
    juce::Slider ampAttackSlider, ampDecaySlider, ampSustainSlider, ampReleaseSlider;
    juce::Label ampAttackLabel, ampDecayLabel, ampSustainLabel, ampReleaseLabel;

    // Filter Envelope
    juce::Slider filtAttackSlider, filtDecaySlider, filtSustainSlider, filtReleaseSlider, filtAmountSlider;
    juce::Label filtAttackLabel, filtDecayLabel, filtSustainLabel, filtReleaseLabel, filtAmountLabel;

    // LFO 1
    juce::ComboBox lfo1WaveCombo;
    juce::Slider lfo1RateSlider, lfo1AmountSlider;
    juce::Label lfo1Label, lfo1RateLabel, lfo1AmountLabel;

    // LFO 2
    juce::ComboBox lfo2WaveCombo;
    juce::Slider lfo2RateSlider, lfo2AmountSlider;
    juce::Label lfo2Label, lfo2RateLabel, lfo2AmountLabel;

    // Mod Matrix
    juce::Label modMatrixLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogSynthAudioProcessorEditor)
};