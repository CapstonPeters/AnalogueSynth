#include "PluginEditor.h"
#include "PluginProcessor.h"

// Helper function for creating rotary sliders
static void setupRotarySlider(juce::Slider& slider, juce::Label& label, const juce::String& paramID,
                              juce::AudioProcessorValueTreeState& apvts,
                              std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment,
                              float min, float max, float interval, float defaultVal,
                              const juce::String& labelText, juce::Component* parent)
{
    parent->addAndMakeVisible(slider);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    slider.setRange(min, max, interval);
    slider.setValue(defaultVal);
    
    parent->addAndMakeVisible(label);
    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(10.0f);
    
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID, slider);
}

// Helper for combo boxes
static void setupComboBox(juce::ComboBox& combo, const juce::String& paramID,
                          juce::AudioProcessorValueTreeState& apvts,
                          std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>& attachment,
                          const juce::StringArray& choices, int defaultIdx,
                          juce::Component* parent)
{
    parent->addAndMakeVisible(combo);
    for (int i = 0; i < choices.size(); ++i) combo.addItem(choices[i], i + 1);
    combo.setSelectedId(defaultIdx + 1);
    
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, paramID, combo);
}

AnalogSynthAudioProcessorEditor::AnalogSynthAudioProcessorEditor (AnalogSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), apvts (p.getAPVTS()),
      testToneButton ("Test Tone"), waveTypeComboBox()
{
    setSize (1000, 700);
    setResizable (true, true);
    
    // === TOP BAR: Test Tone + Global Wave Type + Master Controls ===
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
    
    setupComboBox(waveTypeComboBox, "osc1Wave", apvts, waveTypeAttachment,
                  {"Sine", "Triangle", "Saw", "Square", "Noise"}, 2, this);
    waveTypeComboBox.setTooltip("Global wave type for quick testing (controls Osc 1)");
    
    // Global controls
    setupRotarySlider(masterGainSlider, masterGainLabel, "masterGain", apvts, masterGainAtt,
                      0.0f, 1.0f, 0.01f, 0.5f, "Gain", this);
    setupRotarySlider(polyphonySlider, polyphonyLabel, "polyphony", apvts, polyphonyAtt,
                      1.0f, 16.0f, 1.0f, 8.0f, "Voices", this);
    setupRotarySlider(pitchBendRangeSlider, pitchBendRangeLabel, "pitchBendRange", apvts, pitchBendRangeAtt,
                      0.0f, 24.0f, 0.5f, 2.0f, "PB Range", this);
    
    // === OSCILLATOR SECTION ===
    // Osc 1
    setupComboBox(osc1WaveCombo, "osc1Wave", apvts, osc1WaveAtt,
                  {"Sine", "Triangle", "Saw", "Square", "Noise"}, 2, this);
    setupRotarySlider(osc1LevelSlider, osc1LevelLabel, "osc1Level", apvts, osc1LevelAtt,
                      0.0f, 1.0f, 0.01f, 0.7f, "Level", this);
    setupRotarySlider(osc1PitchSlider, osc1PitchLabel, "osc1Pitch", apvts, osc1PitchAtt,
                      -24.0f, 24.0f, 1.0f, 0.0f, "Pitch", this);
    setupRotarySlider(osc1FineSlider, osc1FineLabel, "osc1FineTune", apvts, osc1FineAtt,
                      -50.0f, 50.0f, 1.0f, 0.0f, "Fine", this);
    setupRotarySlider(osc1PanSlider, osc1PanLabel, "osc1Pan", apvts, osc1PanAtt,
                      -1.0f, 1.0f, 0.01f, 0.0f, "Pan", this);
    setupRotarySlider(osc1UnisonSlider, osc1UnisonLabel, "osc1Unison", apvts, osc1UnisonAtt,
                      1.0f, 8.0f, 1.0f, 1.0f, "Unison", this);
    setupRotarySlider(osc1DetuneSlider, osc1DetuneLabel, "osc1Detune", apvts, osc1DetuneAtt,
                      0.0f, 50.0f, 1.0f, 0.0f, "Detune", this);
    setupRotarySlider(osc1PWSlder, osc1PWLabel, "osc1PulseWidth", apvts, osc1PWAtt,
                      0.01f, 0.99f, 0.01f, 0.5f, "PW", this);
    
    // Osc 2
    setupComboBox(osc2WaveCombo, "osc2Wave", apvts, osc2WaveAtt,
                  {"Sine", "Triangle", "Saw", "Square", "Noise"}, 2, this);
    setupRotarySlider(osc2LevelSlider, osc2LevelLabel, "osc2Level", apvts, osc2LevelAtt,
                      0.0f, 1.0f, 0.01f, 0.5f, "Level", this);
    setupRotarySlider(osc2PitchSlider, osc2PitchLabel, "osc2Pitch", apvts, osc2PitchAtt,
                      -24.0f, 24.0f, 1.0f, 0.0f, "Pitch", this);
    setupRotarySlider(osc2FineSlider, osc2FineLabel, "osc2FineTune", apvts, osc2FineAtt,
                      -50.0f, 50.0f, 1.0f, 0.0f, "Fine", this);
    setupRotarySlider(osc2PanSlider, osc2PanLabel, "osc2Pan", apvts, osc2PanAtt,
                      -1.0f, 1.0f, 0.01f, 0.0f, "Pan", this);
    setupRotarySlider(osc2UnisonSlider, osc2UnisonLabel, "osc2Unison", apvts, osc2UnisonAtt,
                      1.0f, 8.0f, 1.0f, 1.0f, "Unison", this);
    setupRotarySlider(osc2DetuneSlider, osc2DetuneLabel, "osc2Detune", apvts, osc2DetuneAtt,
                      0.0f, 50.0f, 1.0f, 0.0f, "Detune", this);
    setupRotarySlider(osc2PWSlder, osc2PWLabel, "osc2PulseWidth", apvts, osc2PWAtt,
                      0.01f, 0.99f, 0.01f, 0.5f, "PW", this);
    
    // Osc 3
    setupComboBox(osc3WaveCombo, "osc3Wave", apvts, osc3WaveAtt,
                  {"Sine", "Triangle", "Saw", "Square", "Noise"}, 2, this);
    setupRotarySlider(osc3LevelSlider, osc3LevelLabel, "osc3Level", apvts, osc3LevelAtt,
                      0.0f, 1.0f, 0.01f, 0.3f, "Level", this);
    setupRotarySlider(osc3PitchSlider, osc3PitchLabel, "osc3Pitch", apvts, osc3PitchAtt,
                      -24.0f, 24.0f, 1.0f, 0.0f, "Pitch", this);
    setupRotarySlider(osc3FineSlider, osc3FineLabel, "osc3FineTune", apvts, osc3FineAtt,
                      -50.0f, 50.0f, 1.0f, 0.0f, "Fine", this);
    setupRotarySlider(osc3PanSlider, osc3PanLabel, "osc3Pan", apvts, osc3PanAtt,
                      -1.0f, 1.0f, 0.01f, 0.0f, "Pan", this);
    setupRotarySlider(osc3UnisonSlider, osc3UnisonLabel, "osc3Unison", apvts, osc3UnisonAtt,
                      1.0f, 8.0f, 1.0f, 1.0f, "Unison", this);
    setupRotarySlider(osc3DetuneSlider, osc3DetuneLabel, "osc3Detune", apvts, osc3DetuneAtt,
                      0.0f, 50.0f, 1.0f, 0.0f, "Detune", this);
    setupRotarySlider(osc3PWSlder, osc3PWLabel, "osc3PulseWidth", apvts, osc3PWAtt,
                      0.01f, 0.99f, 0.01f, 0.5f, "PW", this);
    
    // Sub
    setupComboBox(subWaveCombo, "subWave", apvts, subWaveAtt,
                  {"Sine", "Square"}, 0, this);
    setupRotarySlider(subLevelSlider, subLabel, "subLevel", apvts, subLevelAtt,
                      0.0f, 1.0f, 0.01f, 0.0f, "Sub Level", this);
    setupRotarySlider(subPitchSlider, subLabel, "subPitch", apvts, subPitchAtt,
                      -24.0f, 0.0f, 1.0f, -12.0f, "Sub Pitch", this);
    
    // Noise
    setupComboBox(noiseWaveCombo, "noiseType", apvts, noiseWaveAtt,
                  {"White", "Pink"}, 0, this);
    setupRotarySlider(noiseLevelSlider, noiseLabel, "noiseLevel", apvts, noiseLevelAtt,
                      0.0f, 1.0f, 0.01f, 0.0f, "Noise Level", this);
    
    // Labels for oscillator section
    addAndMakeVisible(osc1Label); osc1Label.setText("OSC 1", juce::dontSendNotification); osc1Label.setJustificationType(juce::Justification::centred); osc1Label.setColour(juce::Label::textColourId, juce::Colour(0xFF00D4AA)); osc1Label.setFont(12.0f);
    addAndMakeVisible(osc2Label); osc2Label.setText("OSC 2", juce::dontSendNotification); osc2Label.setJustificationType(juce::Justification::centred); osc2Label.setColour(juce::Label::textColourId, juce::Colour(0xFF00D4AA)); osc2Label.setFont(12.0f);
    addAndMakeVisible(osc3Label); osc3Label.setText("OSC 3", juce::dontSendNotification); osc3Label.setJustificationType(juce::Justification::centred); osc3Label.setColour(juce::Label::textColourId, juce::Colour(0xFF00D4AA)); osc3Label.setFont(12.0f);
    addAndMakeVisible(subLabel); subLabel.setText("SUB", juce::dontSendNotification); subLabel.setJustificationType(juce::Justification::centred); subLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF00D4AA)); subLabel.setFont(12.0f);
    addAndMakeVisible(noiseLabel); noiseLabel.setText("NOISE", juce::dontSendNotification); noiseLabel.setJustificationType(juce::Justification::centred); noiseLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF00D4AA)); noiseLabel.setFont(12.0f);
    
    // === FILTER SECTION ===
    setupComboBox(filterTypeCombo, "filterType", apvts, filterTypeAtt,
                  {"LP 4-Pole", "LP 2-Pole", "HP 4-Pole", "HP 2-Pole", "Bandpass", "Notch"}, 0, this);
    setupRotarySlider(cutoffSlider, cutoffLabel, "filterCutoff", apvts, cutoffAtt,
                      20.0f, 20000.0f, 1.0f, 1000.0f, "Cutoff", this);
    setupRotarySlider(resonanceSlider, resonanceLabel, "filterResonance", apvts, resonanceAtt,
                      0.0f, 1.0f, 0.01f, 0.0f, "Reso", this);
    setupRotarySlider(driveSlider, driveLabel, "filterDrive", apvts, driveAtt,
                      0.0f, 1.0f, 0.01f, 0.0f, "Drive", this);
    setupRotarySlider(keyTrackSlider, keyTrackLabel, "filterKeyTrack", apvts, keyTrackAtt,
                      -1.0f, 1.0f, 0.01f, 0.0f, "KeyTrk", this);
    setupRotarySlider(velTrackSlider, velTrackLabel, "filterVelTrack", apvts, velTrackAtt,
                      -1.0f, 1.0f, 0.01f, 0.0f, "VelTrk", this);
    
    addAndMakeVisible(filterLabel); filterLabel.setText("FILTER", juce::dontSendNotification); filterLabel.setJustificationType(juce::Justification::centred); filterLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFFF8844)); filterLabel.setFont(12.0f);
    
    // === AMP ENVELOPE ===
    setupRotarySlider(ampAttackSlider, ampAttackLabel, "ampAttack", apvts, ampAttackAtt,
                      0.001f, 10.0f, 0.001f, 0.01f, "Att", this);
    setupRotarySlider(ampDecaySlider, ampDecayLabel, "ampDecay", apvts, ampDecayAtt,
                      0.001f, 10.0f, 0.001f, 0.3f, "Dec", this);
    setupRotarySlider(ampSustainSlider, ampSustainLabel, "ampSustain", apvts, ampSustainAtt,
                      0.0f, 1.0f, 0.01f, 0.7f, "Sus", this);
    setupRotarySlider(ampReleaseSlider, ampReleaseLabel, "ampRelease", apvts, ampReleaseAtt,
                      0.001f, 10.0f, 0.001f, 0.3f, "Rel", this);
    setupRotarySlider(ampVelSensSlider, ampVelSensLabel, "ampVelSens", apvts, ampVelSensAtt,
                      0.0f, 1.0f, 0.01f, 0.5f, "Vel", this);
    
    addAndMakeVisible(ampAttackLabel); ampAttackLabel.setText("AMP ENV", juce::dontSendNotification); ampAttackLabel.setJustificationType(juce::Justification::centred); ampAttackLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF88AAFF)); ampAttackLabel.setFont(12.0f);
    
    // === FILTER ENVELOPE ===
    setupRotarySlider(filtAttackSlider, filtAttackLabel, "filtAttack", apvts, filtAttackAtt,
                      0.001f, 10.0f, 0.001f, 0.01f, "Att", this);
    setupRotarySlider(filtDecaySlider, filtDecayLabel, "filtDecay", apvts, filtDecayAtt,
                      0.001f, 10.0f, 0.001f, 0.3f, "Dec", this);
    setupRotarySlider(filtSustainSlider, filtSustainLabel, "filtSustain", apvts, filtSustainAtt,
                      0.0f, 1.0f, 0.01f, 0.0f, "Sus", this);
    setupRotarySlider(filtReleaseSlider, filtReleaseLabel, "filtRelease", apvts, filtReleaseAtt,
                      0.001f, 10.0f, 0.001f, 0.3f, "Rel", this);
    setupRotarySlider(filtAmountSlider, filtAmountLabel, "filtAmount", apvts, filtAmountAtt,
                      -1.0f, 1.0f, 0.01f, 0.5f, "Amt", this);
    setupRotarySlider(filtVelSensSlider, filtVelSensLabel, "filtVelSens", apvts, filtVelSensAtt,
                      0.0f, 1.0f, 0.01f, 0.0f, "Vel", this);
    
    addAndMakeVisible(filtAttackLabel); filtAttackLabel.setText("FILT ENV", juce::dontSendNotification); filtAttackLabel.setJustificationType(juce::Justification::centred); filtAttackLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFAA88FF)); filtAttackLabel.setFont(12.0f);
    
    // === LFO 1 ===
    setupComboBox(lfo1WaveCombo, "lfo1Wave", apvts, lfo1WaveAtt,
                  {"Sine", "Triangle", "Saw", "Square", "S&H"}, 0, this);
    setupRotarySlider(lfo1RateSlider, lfo1RateLabel, "lfo1Rate", apvts, lfo1RateAtt,
                      0.01f, 20.0f, 0.01f, 1.0f, "Rate", this);
    setupRotarySlider(lfo1AmountSlider, lfo1AmountLabel, "lfo1Amount", apvts, lfo1AmountAtt,
                      0.0f, 1.0f, 0.01f, 0.0f, "Amt", this);
    setupRotarySlider(lfo1DelaySlider, lfo1DelayLabel, "lfo1Delay", apvts, lfo1DelayAtt,
                      0.0f, 5.0f, 0.01f, 0.0f, "Delay", this);
    setupRotarySlider(lfo1FadeSlider, lfo1FadeLabel, "lfo1Fade", apvts, lfo1FadeAtt,
                      0.0f, 5.0f, 0.01f, 0.0f, "Fade", this);
    
    addAndMakeVisible(lfo1Label); lfo1Label.setText("LFO 1", juce::dontSendNotification); lfo1Label.setJustificationType(juce::Justification::centred); lfo1Label.setColour(juce::Label::textColourId, juce::Colour(0xFFFFAA88)); lfo1Label.setFont(12.0f);
    
    // === LFO 2 ===
    setupComboBox(lfo2WaveCombo, "lfo2Wave", apvts, lfo2WaveAtt,
                  {"Sine", "Triangle", "Saw", "Square", "S&H"}, 1, this);
    setupRotarySlider(lfo2RateSlider, lfo2RateLabel, "lfo2Rate", apvts, lfo2RateAtt,
                      0.01f, 20.0f, 0.01f, 5.0f, "Rate", this);
    setupRotarySlider(lfo2AmountSlider, lfo2AmountLabel, "lfo2Amount", apvts, lfo2AmountAtt,
                      0.0f, 1.0f, 0.01f, 0.0f, "Amt", this);
    setupRotarySlider(lfo2DelaySlider, lfo2DelayLabel, "lfo2Delay", apvts, lfo2DelayAtt,
                      0.0f, 5.0f, 0.01f, 0.0f, "Delay", this);
    setupRotarySlider(lfo2FadeSlider, lfo2FadeLabel, "lfo2Fade", apvts, lfo2FadeAtt,
                      0.0f, 5.0f, 0.01f, 0.0f, "Fade", this);
    
    addAndMakeVisible(lfo2Label); lfo2Label.setText("LFO 2", juce::dontSendNotification); lfo2Label.setJustificationType(juce::Justification::centred); lfo2Label.setColour(juce::Label::textColourId, juce::Colour(0xFFFFAA88)); lfo2Label.setFont(12.0f);
    
    // === MOD MATRIX ===
    addAndMakeVisible(modMatrixLabel);
    modMatrixLabel.setText("MOD MATRIX (8 slots) - Configure via right-click menu or MIDI learn", juce::dontSendNotification);
    modMatrixLabel.setJustificationType(juce::Justification::centred);
    modMatrixLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    modMatrixLabel.setFont(11.0f);
}

AnalogSynthAudioProcessorEditor::~AnalogSynthAudioProcessorEditor() = default;

void AnalogSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xFF1E1E2E));
    
    auto bounds = getLocalBounds();
    auto headerArea = bounds.removeFromTop(50);
    
    // Header
    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawText ("AnalogSynth", headerArea.removeFromLeft(200), juce::Justification::centredLeft);
    
    // Test tone status
    g.setFont (12.0f);
    g.setColour (juce::Colour(0xFF888888));
    g.drawText (processorRef.isTestToneActive() ? "TEST TONE: ON" : "TEST TONE: OFF", headerArea.removeFromRight(200), juce::Justification::centredRight);
    
    // Section dividers
    g.setColour (juce::Colour(0xFF333344));
    for (int y : {150, 320, 420, 520, 620})
        g.drawLine(10, y, getWidth() - 10, y, 1.0f);
}

void AnalogSynthAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    const int knobSize = 64;
    const int knobGap = 4;
    const int labelHeight = 18;
    const int comboHeight = 24;
    const int sectionGap = 10;
    
    // === TOP BAR ===
    auto topBar = bounds.removeFromTop(50);
    testToneButton.setBounds(topBar.removeFromLeft(100).reduced(5));
    waveTypeComboBox.setBounds(topBar.removeFromRight(120).reduced(5));
    
    auto globalArea = topBar.reduced(20, 5);
    masterGainSlider.setBounds(globalArea.removeFromLeft(knobSize).reduced(2));
    masterGainLabel.setBounds(globalArea.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(masterGainSlider.getBottom()));
    polyphonySlider.setBounds(globalArea.removeFromLeft(knobSize).reduced(2));
    polyphonyLabel.setBounds(globalArea.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(polyphonySlider.getBottom()));
    pitchBendRangeSlider.setBounds(globalArea.removeFromLeft(knobSize).reduced(2));
    pitchBendRangeLabel.setBounds(globalArea.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(pitchBendRangeSlider.getBottom()));
    
    bounds.removeFromTop(sectionGap);
    
    // === OSCILLATORS ROW ===
    auto oscSection = bounds.removeFromTop(160);
    oscSection.removeFromTop(20); // Section label space
    
    auto oscRow = oscSection.reduced(10, 5);
    const int oscColWidth = (oscRow.getWidth() - 4 * knobGap) / 5;
    
    // Osc 1 column
    auto osc1Col = oscRow.removeFromLeft(oscColWidth);
    osc1Label.setBounds(osc1Col.removeFromTop(20));
    osc1WaveCombo.setBounds(osc1Col.removeFromTop(comboHeight).reduced(2));
    auto osc1Knobs = osc1Col;
    osc1Knobs.removeFromTop(2);
    osc1LevelSlider.setBounds(osc1Knobs.removeFromLeft(knobSize).reduced(2));
    osc1LevelLabel.setBounds(osc1Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc1LevelSlider.getBottom()));
    osc1PitchSlider.setBounds(osc1Knobs.removeFromLeft(knobSize).reduced(2));
    osc1PitchLabel.setBounds(osc1Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc1PitchSlider.getBottom()));
    osc1FineSlider.setBounds(osc1Knobs.removeFromLeft(knobSize).reduced(2));
    osc1FineLabel.setBounds(osc1Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc1FineSlider.getBottom()));
    osc1PanSlider.setBounds(osc1Knobs.removeFromLeft(knobSize).reduced(2));
    osc1PanLabel.setBounds(osc1Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc1PanSlider.getBottom()));
    osc1UnisonSlider.setBounds(osc1Knobs.removeFromLeft(knobSize).reduced(2));
    osc1UnisonLabel.setBounds(osc1Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc1UnisonSlider.getBottom()));
    osc1DetuneSlider.setBounds(osc1Knobs.removeFromLeft(knobSize).reduced(2));
    osc1DetuneLabel.setBounds(osc1Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc1DetuneSlider.getBottom()));
    osc1PWSlder.setBounds(osc1Knobs.removeFromLeft(knobSize).reduced(2));
    osc1PWLabel.setBounds(osc1Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc1PWSlder.getBottom()));
    
    oscRow.removeFromLeft(knobGap);
    
    // Osc 2 column
    auto osc2Col = oscRow.removeFromLeft(oscColWidth);
    osc2Label.setBounds(osc2Col.removeFromTop(20));
    osc2WaveCombo.setBounds(osc2Col.removeFromTop(comboHeight).reduced(2));
    auto osc2Knobs = osc2Col;
    osc2Knobs.removeFromTop(2);
    osc2LevelSlider.setBounds(osc2Knobs.removeFromLeft(knobSize).reduced(2));
    osc2LevelLabel.setBounds(osc2Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc2LevelSlider.getBottom()));
    osc2PitchSlider.setBounds(osc2Knobs.removeFromLeft(knobSize).reduced(2));
    osc2PitchLabel.setBounds(osc2Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc2PitchSlider.getBottom()));
    osc2FineSlider.setBounds(osc2Knobs.removeFromLeft(knobSize).reduced(2));
    osc2FineLabel.setBounds(osc2Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc2FineSlider.getBottom()));
    osc2PanSlider.setBounds(osc2Knobs.removeFromLeft(knobSize).reduced(2));
    osc2PanLabel.setBounds(osc2Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc2PanSlider.getBottom()));
    osc2UnisonSlider.setBounds(osc2Knobs.removeFromLeft(knobSize).reduced(2));
    osc2UnisonLabel.setBounds(osc2Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc2UnisonSlider.getBottom()));
    osc2DetuneSlider.setBounds(osc2Knobs.removeFromLeft(knobSize).reduced(2));
    osc2DetuneLabel.setBounds(osc2Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc2DetuneSlider.getBottom()));
    osc2PWSlder.setBounds(osc2Knobs.removeFromLeft(knobSize).reduced(2));
    osc2PWLabel.setBounds(osc2Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc2PWSlder.getBottom()));
    
    oscRow.removeFromLeft(knobGap);
    
    // Osc 3 column
    auto osc3Col = oscRow.removeFromLeft(oscColWidth);
    osc3Label.setBounds(osc3Col.removeFromTop(20));
    osc3WaveCombo.setBounds(osc3Col.removeFromTop(comboHeight).reduced(2));
    auto osc3Knobs = osc3Col;
    osc3Knobs.removeFromTop(2);
    osc3LevelSlider.setBounds(osc3Knobs.removeFromLeft(knobSize).reduced(2));
    osc3LevelLabel.setBounds(osc3Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc3LevelSlider.getBottom()));
    osc3PitchSlider.setBounds(osc3Knobs.removeFromLeft(knobSize).reduced(2));
    osc3PitchLabel.setBounds(osc3Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc3PitchSlider.getBottom()));
    osc3FineSlider.setBounds(osc3Knobs.removeFromLeft(knobSize).reduced(2));
    osc3FineLabel.setBounds(osc3Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc3FineSlider.getBottom()));
    osc3PanSlider.setBounds(osc3Knobs.removeFromLeft(knobSize).reduced(2));
    osc3PanLabel.setBounds(osc3Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc3PanSlider.getBottom()));
    osc3UnisonSlider.setBounds(osc3Knobs.removeFromLeft(knobSize).reduced(2));
    osc3UnisonLabel.setBounds(osc3Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc3UnisonSlider.getBottom()));
    osc3DetuneSlider.setBounds(osc3Knobs.removeFromLeft(knobSize).reduced(2));
    osc3DetuneLabel.setBounds(osc3Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc3DetuneSlider.getBottom()));
    osc3PWSlder.setBounds(osc3Knobs.removeFromLeft(knobSize).reduced(2));
    osc3PWLabel.setBounds(osc3Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(osc3PWSlder.getBottom()));
    
    oscRow.removeFromLeft(knobGap);
    
    // Sub + Noise column
    auto subCol = oscRow.removeFromLeft(oscColWidth);
    subLabel.setBounds(subCol.removeFromTop(20));
    subWaveCombo.setBounds(subCol.removeFromTop(comboHeight).reduced(2));
    auto subKnobs = subCol;
    subKnobs.removeFromTop(2);
    // Sub level
    subLevelSlider.setBounds(subKnobs.removeFromLeft(knobSize).reduced(2));
    subLabel.setBounds(subKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(subLevelSlider.getBottom()));
    subPitchSlider.setBounds(subKnobs.removeFromLeft(knobSize).reduced(2));
    subLabel.setBounds(subKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(subPitchSlider.getBottom()));
    
    subCol.removeFromTop(10);
    noiseLabel.setBounds(subCol.removeFromTop(20));
    noiseWaveCombo.setBounds(subCol.removeFromTop(comboHeight).reduced(2));
    auto noiseKnobs = subCol;
    noiseKnobs.removeFromTop(2);
    noiseLevelSlider.setBounds(noiseKnobs.removeFromLeft(knobSize).reduced(2));
    noiseLabel.setBounds(noiseKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(noiseLevelSlider.getBottom()));
    
    bounds.removeFromTop(sectionGap);
    
    // === FILTER SECTION ===
    auto filtSection = bounds.removeFromTop(100);
    filtSection.removeFromTop(20);
    auto filtRow = filtSection.reduced(10, 5);
    filterLabel.setBounds(filtRow.removeFromLeft(120).removeFromTop(20));
    filterTypeCombo.setBounds(filtRow.removeFromLeft(120).removeFromTop(comboHeight).reduced(2));
    
    auto filtKnobs = filtRow;
    cutoffSlider.setBounds(filtKnobs.removeFromLeft(knobSize).reduced(2));
    cutoffLabel.setBounds(filtKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(cutoffSlider.getBottom()));
    resonanceSlider.setBounds(filtKnobs.removeFromLeft(knobSize).reduced(2));
    resonanceLabel.setBounds(filtKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(resonanceSlider.getBottom()));
    driveSlider.setBounds(filtKnobs.removeFromLeft(knobSize).reduced(2));
    driveLabel.setBounds(filtKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(driveSlider.getBottom()));
    keyTrackSlider.setBounds(filtKnobs.removeFromLeft(knobSize).reduced(2));
    keyTrackLabel.setBounds(filtKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(keyTrackSlider.getBottom()));
    velTrackSlider.setBounds(filtKnobs.removeFromLeft(knobSize).reduced(2));
    velTrackLabel.setBounds(filtKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(velTrackSlider.getBottom()));
    
    bounds.removeFromTop(sectionGap);
    
    // === AMP ENVELOPE ===
    auto ampSection = bounds.removeFromTop(100);
    ampSection.removeFromTop(20);
    auto ampRow = ampSection.reduced(10, 5);
    ampAttackLabel.setBounds(ampRow.removeFromLeft(100).removeFromTop(20));
    
    auto ampKnobs = ampRow;
    ampAttackSlider.setBounds(ampKnobs.removeFromLeft(knobSize).reduced(2));
    ampAttackLabel.setBounds(ampKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(ampAttackSlider.getBottom()));
    ampDecaySlider.setBounds(ampKnobs.removeFromLeft(knobSize).reduced(2));
    ampDecayLabel.setBounds(ampKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(ampDecaySlider.getBottom()));
    ampSustainSlider.setBounds(ampKnobs.removeFromLeft(knobSize).reduced(2));
    ampSustainLabel.setBounds(ampKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(ampSustainSlider.getBottom()));
    ampReleaseSlider.setBounds(ampKnobs.removeFromLeft(knobSize).reduced(2));
    ampReleaseLabel.setBounds(ampKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(ampReleaseSlider.getBottom()));
    ampVelSensSlider.setBounds(ampKnobs.removeFromLeft(knobSize).reduced(2));
    ampVelSensLabel.setBounds(ampKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(ampVelSensSlider.getBottom()));
    
    bounds.removeFromTop(sectionGap);
    
    // === FILTER ENVELOPE ===
    auto feSection = bounds.removeFromTop(100);
    feSection.removeFromTop(20);
    auto feRow = feSection.reduced(10, 5);
    filtAttackLabel.setBounds(feRow.removeFromLeft(100).removeFromTop(20));
    
    auto feKnobs = feRow;
    filtAttackSlider.setBounds(feKnobs.removeFromLeft(knobSize).reduced(2));
    filtAttackLabel.setBounds(feKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(filtAttackSlider.getBottom()));
    filtDecaySlider.setBounds(feKnobs.removeFromLeft(knobSize).reduced(2));
    filtDecayLabel.setBounds(feKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(filtDecaySlider.getBottom()));
    filtSustainSlider.setBounds(feKnobs.removeFromLeft(knobSize).reduced(2));
    filtSustainLabel.setBounds(feKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(filtSustainSlider.getBottom()));
    filtReleaseSlider.setBounds(feKnobs.removeFromLeft(knobSize).reduced(2));
    filtReleaseLabel.setBounds(feKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(filtReleaseSlider.getBottom()));
    filtAmountSlider.setBounds(feKnobs.removeFromLeft(knobSize).reduced(2));
    filtAmountLabel.setBounds(feKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(filtAmountSlider.getBottom()));
    filtVelSensSlider.setBounds(feKnobs.removeFromLeft(knobSize).reduced(2));
    filtVelSensLabel.setBounds(feKnobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(filtVelSensSlider.getBottom()));
    
    bounds.removeFromTop(sectionGap);
    
    // === LFOs ===
    auto lfoSection = bounds.removeFromTop(120);
    lfoSection.removeFromTop(20);
    auto lfoRow = lfoSection.reduced(10, 5);
    const int lfoColWidth = lfoRow.getWidth() / 2;
    
    // LFO 1
    auto lfo1Col = lfoRow.removeFromLeft(lfoColWidth);
    lfo1Label.setBounds(lfo1Col.removeFromTop(20));
    lfo1WaveCombo.setBounds(lfo1Col.removeFromTop(comboHeight).reduced(2));
    auto lfo1Knobs = lfo1Col;
    lfo1Knobs.removeFromTop(2);
    lfo1RateSlider.setBounds(lfo1Knobs.removeFromLeft(knobSize).reduced(2));
    lfo1RateLabel.setBounds(lfo1Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(lfo1RateSlider.getBottom()));
    lfo1AmountSlider.setBounds(lfo1Knobs.removeFromLeft(knobSize).reduced(2));
    lfo1AmountLabel.setBounds(lfo1Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(lfo1AmountSlider.getBottom()));
    lfo1DelaySlider.setBounds(lfo1Knobs.removeFromLeft(knobSize).reduced(2));
    lfo1DelayLabel.setBounds(lfo1Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(lfo1DelaySlider.getBottom()));
    lfo1FadeSlider.setBounds(lfo1Knobs.removeFromLeft(knobSize).reduced(2));
    lfo1FadeLabel.setBounds(lfo1Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(lfo1FadeSlider.getBottom()));
    
    // LFO 2
    auto lfo2Col = lfoRow;
    lfo2Label.setBounds(lfo2Col.removeFromTop(20));
    lfo2WaveCombo.setBounds(lfo2Col.removeFromTop(comboHeight).reduced(2));
    auto lfo2Knobs = lfo2Col;
    lfo2Knobs.removeFromTop(2);
    lfo2RateSlider.setBounds(lfo2Knobs.removeFromLeft(knobSize).reduced(2));
    lfo2RateLabel.setBounds(lfo2Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(lfo2RateSlider.getBottom()));
    lfo2AmountSlider.setBounds(lfo2Knobs.removeFromLeft(knobSize).reduced(2));
    lfo2AmountLabel.setBounds(lfo2Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(lfo2AmountSlider.getBottom()));
    lfo2DelaySlider.setBounds(lfo2Knobs.removeFromLeft(knobSize).reduced(2));
    lfo2DelayLabel.setBounds(lfo2Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(lfo2DelaySlider.getBottom()));
    lfo2FadeSlider.setBounds(lfo2Knobs.removeFromLeft(knobSize).reduced(2));
    lfo2FadeLabel.setBounds(lfo2Knobs.removeFromLeft(knobSize).reduced(2).withHeight(labelHeight).withY(lfo2FadeSlider.getBottom()));
    
    bounds.removeFromTop(sectionGap);
    
    // === MOD MATRIX LABEL ===
    auto modSection = bounds.removeFromTop(30);
    modMatrixLabel.setBounds(modSection.reduced(10, 5));
}