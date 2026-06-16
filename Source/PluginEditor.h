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
    juce::AudioProcessorValueTreeState& apvts;
    
    // Test tone controls
    juce::TextButton testToneButton;
    juce::ComboBox waveTypeComboBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveTypeAttachment;
    
    // Oscillators
    juce::ComboBox osc1WaveCombo, osc2WaveCombo, osc3WaveCombo, subWaveCombo, noiseWaveCombo;
    juce::Slider osc1LevelSlider, osc2LevelSlider, osc3LevelSlider;
    juce::Slider osc1PitchSlider, osc2PitchSlider, osc3PitchSlider;
    juce::Slider osc1FineSlider, osc2FineSlider, osc3FineSlider;
    juce::Slider osc1PanSlider, osc2PanSlider, osc3PanSlider;
    juce::Slider osc1UnisonSlider, osc2UnisonSlider, osc3UnisonSlider;
    juce::Slider osc1DetuneSlider, osc2DetuneSlider, osc3DetuneSlider;
    juce::Slider osc1PWSlder, osc2PWSlder, osc3PWSlder;
    
    juce::Label osc1Label, osc2Label, osc3Label, subLabel, noiseLabel;
    juce::Label osc1LevelLabel, osc2LevelLabel, osc3LevelLabel;
    juce::Label osc1PitchLabel, osc2PitchLabel, osc3PitchLabel;
    juce::Label osc1FineLabel, osc2FineLabel, osc3FineLabel;
    juce::Label osc1PanLabel, osc2PanLabel, osc3PanLabel;
    juce::Label osc1UnisonLabel, osc2UnisonLabel, osc3UnisonLabel;
    juce::Label osc1DetuneLabel, osc2DetuneLabel, osc3DetuneLabel;
    juce::Label osc1PWLabel, osc2PWLabel, osc3PWLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> osc1WaveAtt, osc2WaveAtt, osc3WaveAtt, subWaveAtt, noiseWaveAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1LevelAtt, osc2LevelAtt, osc3LevelAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1PitchAtt, osc2PitchAtt, osc3PitchAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1FineAtt, osc2FineAtt, osc3FineAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1PanAtt, osc2PanAtt, osc3PanAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1UnisonAtt, osc2UnisonAtt, osc3UnisonAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1DetuneAtt, osc2DetuneAtt, osc3DetuneAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1PWAtt, osc2PWAtt, osc3PWAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> subLevelAtt, subPitchAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseLevelAtt;
    
    // Sub/Noise sliders
    juce::Slider subLevelSlider, subPitchSlider;
    juce::Slider noiseLevelSlider;
    
    // Filter
    juce::ComboBox filterTypeCombo;
    juce::Slider cutoffSlider, resonanceSlider, driveSlider;
    juce::Slider keyTrackSlider, velTrackSlider;
    juce::Label filterLabel, cutoffLabel, resonanceLabel, driveLabel, keyTrackLabel, velTrackLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterTypeAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAtt, resonanceAtt, driveAtt, keyTrackAtt, velTrackAtt;
    
    // Amp Envelope
    juce::Slider ampAttackSlider, ampDecaySlider, ampSustainSlider, ampReleaseSlider, ampVelSensSlider;
    juce::Label ampAttackLabel, ampDecayLabel, ampSustainLabel, ampReleaseLabel, ampVelSensLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ampAttackAtt, ampDecayAtt, ampSustainAtt, ampReleaseAtt, ampVelSensAtt;
    
    // Filter Envelope
    juce::Slider filtAttackSlider, filtDecaySlider, filtSustainSlider, filtReleaseSlider, filtAmountSlider, filtVelSensSlider;
    juce::Label filtAttackLabel, filtDecayLabel, filtSustainLabel, filtReleaseLabel, filtAmountLabel, filtVelSensLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filtAttackAtt, filtDecayAtt, filtSustainAtt, filtReleaseAtt, filtAmountAtt, filtVelSensAtt;
    
    // LFO 1
    juce::ComboBox lfo1WaveCombo;
    juce::Slider lfo1RateSlider, lfo1AmountSlider, lfo1DelaySlider, lfo1FadeSlider;
    juce::Label lfo1Label, lfo1RateLabel, lfo1AmountLabel, lfo1DelayLabel, lfo1FadeLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfo1WaveAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfo1RateAtt, lfo1AmountAtt, lfo1DelayAtt, lfo1FadeAtt;
    
    // LFO 2
    juce::ComboBox lfo2WaveCombo;
    juce::Slider lfo2RateSlider, lfo2AmountSlider, lfo2DelaySlider, lfo2FadeSlider;
    juce::Label lfo2Label, lfo2RateLabel, lfo2AmountLabel, lfo2DelayLabel, lfo2FadeLabel;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfo2WaveAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfo2RateAtt, lfo2AmountAtt, lfo2DelayAtt, lfo2FadeAtt;
    
    // Global
    juce::Slider masterGainSlider, polyphonySlider, pitchBendRangeSlider;
    juce::Label masterGainLabel, polyphonyLabel, pitchBendRangeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterGainAtt, polyphonyAtt, pitchBendRangeAtt;
    
    // Mod Matrix (simplified - label only for now)
    juce::Label modMatrixLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogSynthAudioProcessorEditor)
};