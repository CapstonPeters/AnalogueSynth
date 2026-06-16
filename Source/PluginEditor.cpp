#include "PluginEditor.h"
#include "PluginProcessor.h"

AnalogSynthAudioProcessorEditor::AnalogSynthAudioProcessorEditor (AnalogSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), testToneButton ("Test Tone"), waveTypeComboBox()
{
    setSize (800, 600);
    setResizable (true, true);
    
    // === TOP ROW: Test Tone + Wave Type ===
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
    waveTypeComboBox.onChange = [this]()
    {
        using WT = TestOscillator::WaveType;
        WT wt = WT::Sine;
        switch (waveTypeComboBox.getSelectedId())
        {
            case 1: wt = WT::Sine; break;
            case 2: wt = WT::Square; break;
            case 3: wt = WT::Saw; break;
        }
        processorRef.setWaveType(wt);
    };
    waveTypeComboBox.setTooltip("Wave type for test tone and voices");

    // === OSCILLATOR SECTION (placeholder - not connected) ===
    addAndMakeVisible(osc1WaveCombo);
    osc1WaveCombo.addItem("Sine", 1); osc1WaveCombo.addItem("Triangle", 2); osc1WaveCombo.addItem("Saw", 3); osc1WaveCombo.addItem("Square", 4); osc1WaveCombo.addItem("Noise", 5);
    osc1WaveCombo.setSelectedId(1);
    addAndMakeVisible(osc1Label); osc1Label.setText("Osc 1", juce::dontSendNotification); osc1Label.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(osc2WaveCombo);
    osc2WaveCombo.addItem("Sine", 1); osc2WaveCombo.addItem("Triangle", 2); osc2WaveCombo.addItem("Saw", 3); osc2WaveCombo.addItem("Square", 4); osc2WaveCombo.addItem("Noise", 5);
    osc2WaveCombo.setSelectedId(3);
    addAndMakeVisible(osc2Label); osc2Label.setText("Osc 2", juce::dontSendNotification); osc2Label.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(osc3WaveCombo);
    osc3WaveCombo.addItem("Sine", 1); osc3WaveCombo.addItem("Triangle", 2); osc3WaveCombo.addItem("Saw", 3); osc3WaveCombo.addItem("Square", 4); osc3WaveCombo.addItem("Noise", 5);
    osc3WaveCombo.setSelectedId(4);
    addAndMakeVisible(osc3Label); osc3Label.setText("Osc 3", juce::dontSendNotification); osc3Label.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(subWaveCombo);
    subWaveCombo.addItem("Sine", 1); subWaveCombo.addItem("Square", 2);
    subWaveCombo.setSelectedId(1);
    addAndMakeVisible(subLabel); subLabel.setText("Sub", juce::dontSendNotification); subLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(noiseWaveCombo);
    noiseWaveCombo.addItem("White", 1); noiseWaveCombo.addItem("Pink", 2);
    noiseWaveCombo.setSelectedId(1);
    addAndMakeVisible(noiseLabel); noiseLabel.setText("Noise", juce::dontSendNotification); noiseLabel.setJustificationType(juce::Justification::centred);

    // === FILTER SECTION (placeholder) ===
    addAndMakeVisible(filterTypeCombo);
    filterTypeCombo.addItem("LP 4-Pole", 1); filterTypeCombo.addItem("LP 2-Pole", 2); filterTypeCombo.addItem("HP 4-Pole", 3); filterTypeCombo.addItem("HP 2-Pole", 4); filterTypeCombo.addItem("BP", 5); filterTypeCombo.addItem("Notch", 6);
    filterTypeCombo.setSelectedId(1);
    addAndMakeVisible(filterLabel); filterLabel.setText("Filter", juce::dontSendNotification); filterLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(cutoffSlider); cutoffSlider.setRange(20.0, 20000.0, 1.0); cutoffSlider.setValue(1000.0); cutoffSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); cutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(cutoffLabel); cutoffLabel.setText("Cutoff", juce::dontSendNotification); cutoffLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(resonanceSlider); resonanceSlider.setRange(0.0, 1.0, 0.01); resonanceSlider.setValue(0.0); resonanceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); resonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(resonanceLabel); resonanceLabel.setText("Resonance", juce::dontSendNotification); resonanceLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(driveSlider); driveSlider.setRange(0.0, 1.0, 0.01); driveSlider.setValue(0.0); driveSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(driveLabel); driveLabel.setText("Drive", juce::dontSendNotification); driveLabel.setJustificationType(juce::Justification::centred);

    // === AMP ENVELOPE SECTION (placeholder) ===
    addAndMakeVisible(ampAttackSlider); ampAttackSlider.setRange(0.001, 10.0, 0.001); ampAttackSlider.setValue(0.01); ampAttackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); ampAttackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(ampAttackLabel); ampAttackLabel.setText("Amp Att", juce::dontSendNotification); ampAttackLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(ampDecaySlider); ampDecaySlider.setRange(0.001, 10.0, 0.001); ampDecaySlider.setValue(0.3); ampDecaySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); ampDecaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(ampDecayLabel); ampDecayLabel.setText("Amp Dec", juce::dontSendNotification); ampDecayLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(ampSustainSlider); ampSustainSlider.setRange(0.0, 1.0, 0.01); ampSustainSlider.setValue(0.7); ampSustainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); ampSustainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(ampSustainLabel); ampSustainLabel.setText("Amp Sus", juce::dontSendNotification); ampSustainLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(ampReleaseSlider); ampReleaseSlider.setRange(0.001, 10.0, 0.001); ampReleaseSlider.setValue(0.3); ampReleaseSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); ampReleaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(ampReleaseLabel); ampReleaseLabel.setText("Amp Rel", juce::dontSendNotification); ampReleaseLabel.setJustificationType(juce::Justification::centred);

    // === FILTER ENVELOPE SECTION (placeholder) ===
    addAndMakeVisible(filtAttackSlider); filtAttackSlider.setRange(0.001, 10.0, 0.001); filtAttackSlider.setValue(0.01); filtAttackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); filtAttackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(filtAttackLabel); filtAttackLabel.setText("Filt Att", juce::dontSendNotification); filtAttackLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(filtDecaySlider); filtDecaySlider.setRange(0.001, 10.0, 0.001); filtDecaySlider.setValue(0.3); filtDecaySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); filtDecaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(filtDecayLabel); filtDecayLabel.setText("Filt Dec", juce::dontSendNotification); filtDecayLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(filtSustainSlider); filtSustainSlider.setRange(0.0, 1.0, 0.01); filtSustainSlider.setValue(0.0); filtSustainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); filtSustainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(filtSustainLabel); filtSustainLabel.setText("Filt Sus", juce::dontSendNotification); filtSustainLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(filtReleaseSlider); filtReleaseSlider.setRange(0.001, 10.0, 0.001); filtReleaseSlider.setValue(0.3); filtReleaseSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); filtReleaseSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(filtReleaseLabel); filtReleaseLabel.setText("Filt Rel", juce::dontSendNotification); filtReleaseLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(filtAmountSlider); filtAmountSlider.setRange(-1.0, 1.0, 0.01); filtAmountSlider.setValue(0.5); filtAmountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); filtAmountSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(filtAmountLabel); filtAmountLabel.setText("Filt Amt", juce::dontSendNotification); filtAmountLabel.setJustificationType(juce::Justification::centred);

    // === LFO 1 SECTION (placeholder) ===
    addAndMakeVisible(lfo1WaveCombo);
    lfo1WaveCombo.addItem("Sine", 1); lfo1WaveCombo.addItem("Triangle", 2); lfo1WaveCombo.addItem("Saw", 3); lfo1WaveCombo.addItem("Square", 4); lfo1WaveCombo.addItem("S&H", 5);
    lfo1WaveCombo.setSelectedId(1);
    addAndMakeVisible(lfo1Label); lfo1Label.setText("LFO 1", juce::dontSendNotification); lfo1Label.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(lfo1RateSlider); lfo1RateSlider.setRange(0.01, 20.0, 0.01); lfo1RateSlider.setValue(1.0); lfo1RateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); lfo1RateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(lfo1RateLabel); lfo1RateLabel.setText("Rate (Hz)", juce::dontSendNotification); lfo1RateLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(lfo1AmountSlider); lfo1AmountSlider.setRange(0.0, 1.0, 0.01); lfo1AmountSlider.setValue(0.0); lfo1AmountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); lfo1AmountSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(lfo1AmountLabel); lfo1AmountLabel.setText("Amount", juce::dontSendNotification); lfo1AmountLabel.setJustificationType(juce::Justification::centred);

    // === LFO 2 SECTION (placeholder) ===
    addAndMakeVisible(lfo2WaveCombo);
    lfo2WaveCombo.addItem("Sine", 1); lfo2WaveCombo.addItem("Triangle", 2); lfo2WaveCombo.addItem("Saw", 3); lfo2WaveCombo.addItem("Square", 4); lfo2WaveCombo.addItem("S&H", 5);
    lfo2WaveCombo.setSelectedId(2);
    addAndMakeVisible(lfo2Label); lfo2Label.setText("LFO 2", juce::dontSendNotification); lfo2Label.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(lfo2RateSlider); lfo2RateSlider.setRange(0.01, 20.0, 0.01); lfo2RateSlider.setValue(5.0); lfo2RateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); lfo2RateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(lfo2RateLabel); lfo2RateLabel.setText("Rate (Hz)", juce::dontSendNotification); lfo2RateLabel.setJustificationType(juce::Justification::centred);
    
    addAndMakeVisible(lfo2AmountSlider); lfo2AmountSlider.setRange(0.0, 1.0, 0.01); lfo2AmountSlider.setValue(0.0); lfo2AmountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); lfo2AmountSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    addAndMakeVisible(lfo2AmountLabel); lfo2AmountLabel.setText("Amount", juce::dontSendNotification); lfo2AmountLabel.setJustificationType(juce::Justification::centred);

    // === MOD MATRIX SECTION (placeholder) ===
    addAndMakeVisible(modMatrixLabel);
    modMatrixLabel.setText("Mod Matrix (8 slots) - Source -> Dest -> Amount", juce::dontSendNotification);
    modMatrixLabel.setJustificationType(juce::Justification::centred);
    modMatrixLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
}

AnalogSynthAudioProcessorEditor::~AnalogSynthAudioProcessorEditor() = default;

void AnalogSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(0xFF1E1E2E));
    
    auto bounds = getLocalBounds();
    auto headerArea = bounds.removeFromTop(60);
    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawText ("AnalogSynth", headerArea, juce::Justification::centred);
    
    g.setFont (12.0f);
    g.setColour (juce::Colour(0xFF888888));
    g.drawText ("Wave: " + waveTypeComboBox.getItemText(waveTypeComboBox.getSelectedItemIndex()), headerArea.reduced(20, 0).removeFromRight(200), juce::Justification::centredRight);
    g.drawText (processorRef.isTestToneActive() ? "Test Tone: ON" : "Test Tone: OFF", headerArea.reduced(20, 0).removeFromLeft(200), juce::Justification::centredLeft);
    
    // Section header text labels
    g.setFont (13.0f);
    
    auto sectionBounds = bounds;
    
    // Oscillators
    auto oscSection = sectionBounds.removeFromTop(80);
    g.setColour (juce::Colour(0xFF00D4AA));
    g.drawText ("OSCILLATORS", oscSection.removeFromTop(20), juce::Justification::centredLeft);
    
    sectionBounds.removeFromTop(10);
    
    // Filter
    auto filtSection = sectionBounds.removeFromTop(100);
    g.setColour (juce::Colour(0xFFFF8844));
    g.drawText ("FILTER", filtSection.removeFromTop(20), juce::Justification::centredLeft);
    
    sectionBounds.removeFromTop(10);
    
    // Amp Envelope
    auto ampSection = sectionBounds.removeFromTop(80);
    g.setColour (juce::Colour(0xFF88AAFF));
    g.drawText ("AMP ENVELOPE", ampSection.removeFromTop(20), juce::Justification::centredLeft);
    
    sectionBounds.removeFromTop(10);
    
    // Filter Envelope
    auto feSection = sectionBounds.removeFromTop(100);
    g.setColour (juce::Colour(0xFFAA88FF));
    g.drawText ("FILTER ENVELOPE", feSection.removeFromTop(20), juce::Justification::centredLeft);
    
    sectionBounds.removeFromTop(10);
    
    // LFOs
    auto lfoSection = sectionBounds.removeFromTop(80);
    g.setColour (juce::Colour(0xFFFFAA88));
    g.drawText ("LFOs", lfoSection.removeFromTop(20), juce::Justification::centredLeft);
    
    sectionBounds.removeFromTop(10);
    
    // Mod Matrix
    auto modSection = sectionBounds.removeFromTop(30);
    g.setColour (juce::Colours::grey);
    g.drawText ("MODULATION MATRIX", modSection.removeFromTop(20), juce::Justification::centredLeft);
}

void AnalogSynthAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Top row: Test Tone button + Wave Type combo
    auto topRow = bounds.removeFromTop(60);
    auto buttonArea = topRow.removeFromLeft(200).reduced(10);
    testToneButton.setBounds(buttonArea);
    
    auto comboArea = topRow.removeFromRight(200).reduced(10);
    waveTypeComboBox.setBounds(comboArea);
    
    bounds.removeFromTop(10); // spacing
    
    // === OSCILLATOR SECTION ===
    auto oscSection = bounds.removeFromTop(80);
    auto oscRow = oscSection.reduced(15, 10);
    auto oscWidth = oscRow.getWidth() / 5;
    osc1WaveCombo.setBounds(oscRow.removeFromLeft(oscWidth).reduced(2).withTrimmedTop(5).withTrimmedBottom(20));
    osc1Label.setBounds(oscRow.removeFromLeft(oscWidth).reduced(2).withTrimmedTop(5).withHeight(15).withY(osc1WaveCombo.getBottom()));
    osc2WaveCombo.setBounds(oscRow.removeFromLeft(oscWidth).reduced(2).withTrimmedTop(5).withTrimmedBottom(20));
    osc2Label.setBounds(oscRow.removeFromLeft(oscWidth).reduced(2).withTrimmedTop(5).withHeight(15).withY(osc2WaveCombo.getBottom()));
    osc3WaveCombo.setBounds(oscRow.removeFromLeft(oscWidth).reduced(2).withTrimmedTop(5).withTrimmedBottom(20));
    osc3Label.setBounds(oscRow.removeFromLeft(oscWidth).reduced(2).withTrimmedTop(5).withHeight(15).withY(osc3WaveCombo.getBottom()));
    
    // Sub and Noise in a sub-row
    auto subRow = oscSection.removeFromBottom(30).reduced(15, 5);
    auto subWidth = subRow.getWidth() / 2;
    subWaveCombo.setBounds(subRow.removeFromLeft(subWidth).reduced(2).withTrimmedTop(5).withTrimmedBottom(5));
    subLabel.setBounds(subRow.removeFromLeft(subWidth).reduced(2).withTrimmedTop(5).withHeight(15).withY(subWaveCombo.getBottom()));
    noiseWaveCombo.setBounds(subRow.removeFromLeft(subWidth).reduced(2).withTrimmedTop(5).withTrimmedBottom(5));
    noiseLabel.setBounds(subRow.removeFromLeft(subWidth).reduced(2).withTrimmedTop(5).withHeight(15).withY(noiseWaveCombo.getBottom()));
    
    bounds.removeFromTop(10); // spacing
    
    // === FILTER SECTION ===
    auto filtSection = bounds.removeFromTop(100);
    auto filtRow = filtSection.reduced(15, 10);
    filterTypeCombo.setBounds(filtRow.removeFromLeft(150).reduced(2).withTrimmedTop(5).withTrimmedBottom(20));
    filterLabel.setBounds(filtRow.removeFromLeft(150).reduced(2).withTrimmedTop(5).withHeight(15).withY(filterTypeCombo.getBottom()));
    
    auto knobWidth = 80;
    cutoffSlider.setBounds(filtRow.removeFromLeft(knobWidth).reduced(5).withTrimmedTop(5));
    cutoffLabel.setBounds(filtRow.removeFromLeft(knobWidth).reduced(5).withHeight(15).withY(cutoffSlider.getBottom() + 2));
    resonanceSlider.setBounds(filtRow.removeFromLeft(knobWidth).reduced(5).withTrimmedTop(5));
    resonanceLabel.setBounds(filtRow.removeFromLeft(knobWidth).reduced(5).withHeight(15).withY(resonanceSlider.getBottom() + 2));
    driveSlider.setBounds(filtRow.removeFromLeft(knobWidth).reduced(5).withTrimmedTop(5));
    driveLabel.setBounds(filtRow.removeFromLeft(knobWidth).reduced(5).withHeight(15).withY(driveSlider.getBottom() + 2));
    
    bounds.removeFromTop(10); // spacing
    
    // === AMP ENVELOPE SECTION ===
    auto ampSection = bounds.removeFromTop(80);
    auto ampRow = ampSection.reduced(15, 10);
    ampAttackSlider.setBounds(ampRow.removeFromLeft(80).reduced(5).withTrimmedTop(5));
    ampAttackLabel.setBounds(ampRow.removeFromLeft(80).reduced(5).withHeight(15).withY(ampAttackSlider.getBottom() + 2));
    ampDecaySlider.setBounds(ampRow.removeFromLeft(80).reduced(5).withTrimmedTop(5));
    ampDecayLabel.setBounds(ampRow.removeFromLeft(80).reduced(5).withHeight(15).withY(ampDecaySlider.getBottom() + 2));
    ampSustainSlider.setBounds(ampRow.removeFromLeft(80).reduced(5).withTrimmedTop(5));
    ampSustainLabel.setBounds(ampRow.removeFromLeft(80).reduced(5).withHeight(15).withY(ampSustainSlider.getBottom() + 2));
    ampReleaseSlider.setBounds(ampRow.removeFromLeft(80).reduced(5).withTrimmedTop(5));
    ampReleaseLabel.setBounds(ampRow.removeFromLeft(80).reduced(5).withHeight(15).withY(ampReleaseSlider.getBottom() + 2));
    
    bounds.removeFromTop(10); // spacing
    
    // === FILTER ENVELOPE SECTION ===
    auto filtEnvSection = bounds.removeFromTop(100);
    auto feRow = filtEnvSection.reduced(15, 10);
    filtAttackSlider.setBounds(feRow.removeFromLeft(80).reduced(5).withTrimmedTop(5));
    filtAttackLabel.setBounds(feRow.removeFromLeft(80).reduced(5).withHeight(15).withY(filtAttackSlider.getBottom() + 2));
    filtDecaySlider.setBounds(feRow.removeFromLeft(80).reduced(5).withTrimmedTop(5));
    filtDecayLabel.setBounds(feRow.removeFromLeft(80).reduced(5).withHeight(15).withY(filtDecaySlider.getBottom() + 2));
    filtSustainSlider.setBounds(feRow.removeFromLeft(80).reduced(5).withTrimmedTop(5));
    filtSustainLabel.setBounds(feRow.removeFromLeft(80).reduced(5).withHeight(15).withY(filtSustainSlider.getBottom() + 2));
    filtReleaseSlider.setBounds(feRow.removeFromLeft(80).reduced(5).withTrimmedTop(5));
    filtReleaseLabel.setBounds(feRow.removeFromLeft(80).reduced(5).withHeight(15).withY(filtReleaseSlider.getBottom() + 2));
    filtAmountSlider.setBounds(feRow.removeFromLeft(80).reduced(5).withTrimmedTop(5));
    filtAmountLabel.setBounds(feRow.removeFromLeft(80).reduced(5).withHeight(15).withY(filtAmountSlider.getBottom() + 2));
    
    bounds.removeFromTop(10); // spacing
    
    // === LFO SECTION ===
    auto lfoSection = bounds.removeFromTop(80);
    auto lfoRow = lfoSection.reduced(15, 10);
    
    auto lfoColWidth = lfoRow.getWidth() / 2;
    auto lfo1Col = lfoRow.removeFromLeft(lfoColWidth);
    lfo1WaveCombo.setBounds(lfo1Col.removeFromTop(30).reduced(5));
    lfo1Label.setBounds(lfo1Col.removeFromTop(20).reduced(5));
    lfo1RateSlider.setBounds(lfo1Col.removeFromTop(50).reduced(5));
    lfo1RateLabel.setBounds(lfo1Col.removeFromTop(20).reduced(5));
    lfo1AmountSlider.setBounds(lfo1Col.removeFromTop(50).reduced(5));
    lfo1AmountLabel.setBounds(lfo1Col.removeFromTop(20).reduced(5));
    
    auto lfo2Col = lfoRow;
    lfo2WaveCombo.setBounds(lfo2Col.removeFromTop(30).reduced(5));
    lfo2Label.setBounds(lfo2Col.removeFromTop(20).reduced(5));
    lfo2RateSlider.setBounds(lfo2Col.removeFromTop(50).reduced(5));
    lfo2RateLabel.setBounds(lfo2Col.removeFromTop(20).reduced(5));
    lfo2AmountSlider.setBounds(lfo2Col.removeFromTop(50).reduced(5));
    lfo2AmountLabel.setBounds(lfo2Col.removeFromTop(20).reduced(5));
    
    bounds.removeFromTop(10); // spacing
    
    // === MOD MATRIX LABEL ===
    auto modSection = bounds.removeFromTop(30);
    modMatrixLabel.setBounds(modSection.reduced(15, 5));
}