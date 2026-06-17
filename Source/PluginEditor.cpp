#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
// Custom LookAndFeel
class SynthLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SynthLookAndFeel()
    {
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFF00D4AA));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF333344));
        setColour(juce::Slider::thumbColourId, juce::Colours::white);
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF252535));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF333344));
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
        setColour(juce::Label::textColourId, juce::Colour(0xFFBBBBBB));
        setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF2ECC71));
        setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFE74C3C));
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    }
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(4);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto centre = bounds.getCentre();
        
        g.setColour(juce::Colour(0xFF1A1A2A));
        g.fillEllipse(bounds);
        
        g.setColour(juce::Colour(0xFF333344));
        g.drawEllipse(bounds, 2.0f);
        
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        juce::Path arc;
        arc.addCentredArc(centre.x, centre.y, radius - 6, radius - 6, 0.0f,
                          rotaryStartAngle, angle, true);
        
        g.setColour(findColour(juce::Slider::rotarySliderFillColourId).withAlpha(0.8f));
        g.strokePath(arc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        juce::Path indicator;
        indicator.addRectangle(-2.0f, -radius + 8, 4.0f, radius - 16);
        g.setColour(juce::Colours::white);
        g.fillPath(indicator, juce::AffineTransform::rotation(angle).translated(centre));
        
        g.setColour(juce::Colour(0xFF1A1A2A));
        g.fillEllipse(centre.x - 6, centre.y - 6, 12, 12);
        g.setColour(juce::Colour(0xFF333344));
        g.drawEllipse(centre.x - 6, centre.y - 6, 12, 12, 1.5f);
    }
    
    juce::Label* createSliderTextBox(juce::Slider& slider) override
    {
        auto l = LookAndFeel_V4::createSliderTextBox(slider);
        l->setColour(juce::Label::outlineColourId, juce::Colour(0x00000000));
        l->setColour(juce::Label::backgroundColourId, juce::Colour(0x801A1A2A));
        l->setFont(11.0f);
        l->setJustificationType(juce::Justification::centred);
        return l;
    }
    
    void drawComboBox(juce::Graphics& g, int width, int height, bool,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override
    {
        juce::Rectangle<int> boxBounds(0, 0, width, height);
        g.setColour(findColour(juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle(boxBounds.toFloat(), 4.0f);
        g.setColour(findColour(juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f), 4.0f, 1.5f);
        
        juce::Path arrow;
        arrow.addTriangle(0, 0, 6, 0, 3, 4);
        g.setColour(findColour(juce::ComboBox::textColourId));
        g.fillPath(arrow, juce::AffineTransform::translation(width - buttonW + (buttonW - 6) / 2.0f,
                                                              (height - 4) / 2.0f));
    }
};

static SynthLookAndFeel synthLookAndFeel;

//==============================================================================
// Helpers
static void styleKnob(juce::Slider& slider, const juce::String& suffix = "")
{
    slider.setLookAndFeel(&synthLookAndFeel);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 54, 20);
    slider.setTextValueSuffix(suffix);
}

static void styleCombo(juce::ComboBox& combo)
{
    combo.setLookAndFeel(&synthLookAndFeel);
}

static void setupCombo(juce::ComboBox& combo, const juce::String& paramID,
                        juce::AudioProcessorValueTreeState& apvts,
                        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>& attachment,
                        const juce::StringArray& choices, int defaultIdx,
                        juce::Component* parent)
{
    if (parent) parent->addAndMakeVisible(combo);
    styleCombo(combo);
    for (int i = 0; i < choices.size(); ++i) combo.addItem(choices[i], i + 1);
    combo.setSelectedId(defaultIdx + 1);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, paramID, combo);
}

//==============================================================================
// Section Panel
class SectionPanel : public juce::Component
{
public:
    SectionPanel(const juce::String& title, juce::Colour accentColour)
        : titleText(title), accent(accentColour) {}
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        juce::ColourGradient gradient(juce::Colour(0xFF222232), bounds.getTopLeft(),
                                      juce::Colour(0xFF1A1A25), bounds.getBottomLeft(), false);
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(bounds.reduced(1), 8.0f);
        
        g.setColour(accent);
        auto accentRect = bounds.removeFromTop(2).reduced(2, 1);
        g.fillRoundedRectangle(accentRect, 8.0f);
        
        g.setColour(juce::Colour(0xFF2A2A3A));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);
        
        g.setColour(accent);
        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.drawText(titleText, getLocalBounds().removeFromTop(28).reduced(12, 0),
                   juce::Justification::centredLeft);
    }
    
private:
    juce::String titleText;
    juce::Colour accent;
};

//==============================================================================
// Knob Group
struct KnobGroup
{
    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    
    void setup(const juce::String& paramID, juce::AudioProcessorValueTreeState& apvts,
               float min, float max, float interval, float def,
               const juce::String& labelText, const juce::String& suffix,
               juce::Component* parent);
    
    void setBounds(juce::Rectangle<int> area, int knobSize, int labelHeight);
};

void AnalogSynthAudioProcessorEditor::KnobGroup::setup(const juce::String& paramID, juce::AudioProcessorValueTreeState& apvts,
                  float min, float max, float interval, float def,
                  const juce::String& labelText, const juce::String& suffix,
                  juce::Component* parent)
{
    if (parent) parent->addAndMakeVisible(slider);
    styleKnob(slider, suffix);
    slider.setRange(min, max, interval);
    slider.setValue(def);
    
    if (parent) parent->addAndMakeVisible(label);
    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(10.0f);
    label.setColour(juce::Label::textColourId, juce::Colour(0xFF888888));
    
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID, slider);
}

void AnalogSynthAudioProcessorEditor::KnobGroup::setBounds(juce::Rectangle<int> area, int knobSize, int labelHeight)
{
    slider.setBounds(area.removeFromTop(knobSize).reduced(2));
    label.setBounds(area.removeFromTop(labelHeight).reduced(2));
}

//==============================================================================
AnalogSynthAudioProcessorEditor::AnalogSynthAudioProcessorEditor (AnalogSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), apvts (p.getAPVTS())
{
    setSize (1100, 780);
    setResizable (true, true);
    
    // === TOP BAR ===
    addAndMakeVisible(testToneButton);
    testToneButton.setButtonText("Test Tone A4");
    testToneButton.setClickingTogglesState(true);
    testToneButton.onClick = [this]()
    {
        processorRef.setTestToneActive(testToneButton.getToggleState());
        testToneButton.setButtonText(testToneButton.getToggleState() ? "Stop Tone" : "Test Tone A4");
        repaint();
    };
    
    waveTypeComboBox.addItemList({"Sine", "Triangle", "Saw", "Square", "Noise"}, 1);
    waveTypeComboBox.setSelectedId(3);
    waveTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "osc1Wave", waveTypeComboBox);
    
    // Global knobs
    masterGainKnob.setup("masterGain", apvts, 0.0f, 1.0f, 0.01f, 0.5f, "GAIN", " dB", this);
    polyphonyKnob.setup("polyphony", apvts, 1.0f, 16.0f, 1.0f, 8.0f, "VOICES", "", this);
    pitchBendKnob.setup("pitchBendRange", apvts, 0.0f, 24.0f, 0.5f, 2.0f, "PB RANGE", " st", this);
    
    // === OSCILLATORS ===
    setupCombo(osc1Wave, "osc1Wave", apvts, osc1WaveAtt, {"Sine", "Triangle", "Saw", "Square", "Noise"}, 2, this);
    osc1Level.setup("osc1Level", apvts, 0.0f, 1.0f, 0.01f, 0.7f, "LEVEL", "", this);
    osc1Pitch.setup("osc1Pitch", apvts, -24.0f, 24.0f, 1.0f, 0.0f, "PITCH", " st", this);
    osc1Fine.setup("osc1FineTune", apvts, -50.0f, 50.0f, 1.0f, 0.0f, "FINE", " ct", this);
    osc1Pan.setup("osc1Pan", apvts, -1.0f, 1.0f, 0.01f, 0.0f, "PAN", "", this);
    osc1Unison.setup("osc1Unison", apvts, 1.0f, 8.0f, 1.0f, 1.0f, "UNISON", "", this);
    osc1Detune.setup("osc1Detune", apvts, 0.0f, 50.0f, 1.0f, 0.0f, "DETUNE", " ct", this);
    osc1PW.setup("osc1PulseWidth", apvts, 0.01f, 0.99f, 0.01f, 0.5f, "PW", "%", this);
    
    setupCombo(osc2Wave, "osc2Wave", apvts, osc2WaveAtt, {"Sine", "Triangle", "Saw", "Square", "Noise"}, 2, this);
    osc2Level.setup("osc2Level", apvts, 0.0f, 1.0f, 0.01f, 0.5f, "LEVEL", "", this);
    osc2Pitch.setup("osc2Pitch", apvts, -24.0f, 24.0f, 1.0f, 0.0f, "PITCH", " st", this);
    osc2Fine.setup("osc2FineTune", apvts, -50.0f, 50.0f, 1.0f, 0.0f, "FINE", " ct", this);
    osc2Pan.setup("osc2Pan", apvts, -1.0f, 1.0f, 0.01f, 0.0f, "PAN", "", this);
    osc2Unison.setup("osc2Unison", apvts, 1.0f, 8.0f, 1.0f, 1.0f, "UNISON", "", this);
    osc2Detune.setup("osc2Detune", apvts, 0.0f, 50.0f, 1.0f, 0.0f, "DETUNE", " ct", this);
    osc2PW.setup("osc2PulseWidth", apvts, 0.01f, 0.99f, 0.01f, 0.5f, "PW", "%", this);
    
    setupCombo(osc3Wave, "osc3Wave", apvts, osc3WaveAtt, {"Sine", "Triangle", "Saw", "Square", "Noise"}, 2, this);
    osc3Level.setup("osc3Level", apvts, 0.0f, 1.0f, 0.01f, 0.3f, "LEVEL", "", this);
    osc3Pitch.setup("osc3Pitch", apvts, -24.0f, 24.0f, 1.0f, 0.0f, "PITCH", " st", this);
    osc3Fine.setup("osc3FineTune", apvts, -50.0f, 50.0f, 1.0f, 0.0f, "FINE", " ct", this);
    osc3Pan.setup("osc3Pan", apvts, -1.0f, 1.0f, 0.01f, 0.0f, "PAN", "", this);
    osc3Unison.setup("osc3Unison", apvts, 1.0f, 8.0f, 1.0f, 1.0f, "UNISON", "", this);
    osc3Detune.setup("osc3Detune", apvts, 0.0f, 50.0f, 1.0f, 0.0f, "DETUNE", " ct", this);
    osc3PW.setup("osc3PulseWidth", apvts, 0.01f, 0.99f, 0.01f, 0.5f, "PW", "%", this);
    
    setupCombo(subWave, "subWave", apvts, subWaveAtt, {"Sine", "Square"}, 0, this);
    subLevel.setup("subLevel", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "SUB LEVEL", "", this);
    subPitch.setup("subPitch", apvts, -24.0f, 0.0f, 1.0f, -12.0f, "SUB PITCH", " st", this);
    
    setupCombo(noiseWave, "noiseType", apvts, noiseWaveAtt, {"White", "Pink"}, 0, this);
    noiseLevel.setup("noiseLevel", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "NOISE LEVEL", "", this);
    
    // === FILTER ===
    setupCombo(filterType, "filterType", apvts, filterTypeAtt, {"LP 4-Pole", "LP 2-Pole", "HP 4-Pole", "HP 2-Pole", "Bandpass", "Notch"}, 0, this);
    filterCutoff.setup("filterCutoff", apvts, 20.0f, 20000.0f, 1.0f, 1000.0f, "CUTOFF", " Hz", this);
    filterReso.setup("filterResonance", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "RESO", "", this);
    filterDrive.setup("filterDrive", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "DRIVE", "", this);
    filterKeyTrk.setup("filterKeyTrack", apvts, -1.0f, 1.0f, 0.01f, 0.0f, "KEY TRK", "", this);
    filterVelTrk.setup("filterVelTrack", apvts, -1.0f, 1.0f, 0.01f, 0.0f, "VEL TRK", "", this);
    
    // === AMP ENV ===
    ampAtt.setup("ampAttack", apvts, 0.001f, 10.0f, 0.001f, 0.01f, "ATT", " s", this);
    ampDec.setup("ampDecay", apvts, 0.001f, 10.0f, 0.001f, 0.3f, "DEC", " s", this);
    ampSus.setup("ampSustain", apvts, 0.0f, 1.0f, 0.01f, 0.7f, "SUS", "", this);
    ampRel.setup("ampRelease", apvts, 0.001f, 10.0f, 0.001f, 0.3f, "REL", " s", this);
    ampVel.setup("ampVelSens", apvts, 0.0f, 1.0f, 0.01f, 0.5f, "VEL", "", this);
    
    // === FILT ENV ===
    filtAtt.setup("filtAttack", apvts, 0.001f, 10.0f, 0.001f, 0.01f, "ATT", " s", this);
    filtDec.setup("filtDecay", apvts, 0.001f, 10.0f, 0.001f, 0.3f, "DEC", " s", this);
    filtSus.setup("filtSustain", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "SUS", "", this);
    filtRel.setup("filtRelease", apvts, 0.001f, 10.0f, 0.001f, 0.3f, "REL", " s", this);
    filtAmt.setup("filtAmount", apvts, -1.0f, 1.0f, 0.01f, 0.5f, "AMT", "", this);
    filtVel.setup("filtVelSens", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "VEL", "", this);
    
    // === LFOs ===
    setupCombo(lfo1Wave, "lfo1Wave", apvts, lfo1WaveAtt, {"Sine", "Triangle", "Saw", "Square", "S&H"}, 0, this);
    lfo1Rate.setup("lfo1Rate", apvts, 0.01f, 20.0f, 0.01f, 1.0f, "RATE", " Hz", this);
    lfo1Amt.setup("lfo1Amount", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "AMT", "", this);
    lfo1Delay.setup("lfo1Delay", apvts, 0.0f, 5.0f, 0.01f, 0.0f, "DELAY", " s", this);
    lfo1Fade.setup("lfo1Fade", apvts, 0.0f, 5.0f, 0.01f, 0.0f, "FADE", " s", this);
    
    setupCombo(lfo2Wave, "lfo2Wave", apvts, lfo2WaveAtt, {"Sine", "Triangle", "Saw", "Square", "S&H"}, 1, this);
    lfo2Rate.setup("lfo2Rate", apvts, 0.01f, 20.0f, 0.01f, 5.0f, "RATE", " Hz", this);
    lfo2Amt.setup("lfo2Amount", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "AMT", "", this);
    lfo2Delay.setup("lfo2Delay", apvts, 0.0f, 5.0f, 0.01f, 0.0f, "DELAY", " s", this);
    lfo2Fade.setup("lfo2Fade", apvts, 0.0f, 5.0f, 0.01f, 0.0f, "FADE", " s", this);
    
    // Section panels
    addAndMakeVisible(oscPanel);
    addAndMakeVisible(filterPanel);
    addAndMakeVisible(ampEnvPanel);
    addAndMakeVisible(filtEnvPanel);
    addAndMakeVisible(lfoPanel);
    addAndMakeVisible(modPanel);
}

AnalogSynthAudioProcessorEditor::~AnalogSynthAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void AnalogSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF15151E));
    
    auto header = getLocalBounds().removeFromTop(56);
    g.setColour(juce::Colour(0xFF1A1A25));
    g.fillRect(header);
    g.setColour(juce::Colour(0xFF2A2A3A));
    g.drawLine(0, header.getBottom(), getWidth(), header.getBottom(), 1.0f);
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("AnalogSynth", header.removeFromLeft(180).reduced(16, 0), juce::Justification::centredLeft);
    
    g.setFont(11.0f);
    g.setColour(juce::Colour(0xFF666677));
    auto statusArea = header.removeFromRight(300);
    g.drawText(processorRef.isTestToneActive() ? "● TEST ACTIVE" : "○ TEST READY", 
               statusArea.reduced(16, 0), juce::Justification::centredRight);
}

void AnalogSynthAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(12);
    const int knobSize = 58;
    const int labelH = 16;
    const int comboH = 28;
    const int sectionGap = 8;
    
    // Header
    auto headerBar = bounds.removeFromTop(56);
    testToneButton.setBounds(headerBar.removeFromLeft(110).reduced(6));
    headerBar.removeFromLeft(12);
    waveTypeComboBox.setBounds(headerBar.removeFromLeft(120).reduced(4).withHeight(comboH).withY(12));
    
    auto globalArea = headerBar.removeFromRight(220).reduced(4);
    masterGainKnob.slider.setBounds(globalArea.removeFromLeft(70).reduced(2));
    masterGainKnob.label.setBounds(globalArea.removeFromLeft(70).reduced(2).withHeight(labelH).withY(masterGainKnob.slider.getBottom()));
    polyphonyKnob.slider.setBounds(globalArea.removeFromLeft(70).reduced(2));
    polyphonyKnob.label.setBounds(globalArea.removeFromLeft(70).reduced(2).withHeight(labelH).withY(polyphonyKnob.slider.getBottom()));
    pitchBendKnob.slider.setBounds(globalArea.removeFromLeft(70).reduced(2));
    pitchBendKnob.label.setBounds(globalArea.removeFromLeft(70).reduced(2).withHeight(labelH).withY(pitchBendKnob.slider.getBottom()));
    
    bounds.removeFromTop(sectionGap);
    
    // 2-column layout
    auto leftCol = bounds.removeFromLeft((bounds.getWidth() * 3) / 5);
    auto rightCol = bounds;
    
    // === LEFT: OSC + FILTER ===
    auto oscArea = leftCol.removeFromTop(340);
    oscPanel.setBounds(oscArea);
    auto oscInner = oscArea.reduced(14, 34);
    
    int oscColW = (oscInner.getWidth() - 4 * 6) / 5;
    
    auto layoutOsc = [&](juce::ComboBox& wave, KnobGroup& level, KnobGroup& pitch, KnobGroup& fine,
                         KnobGroup& pan, KnobGroup& unison, KnobGroup& detune, KnobGroup& pw,
                         juce::Rectangle<int> col)
    {
        wave.setBounds(col.removeFromTop(comboH).reduced(2));
        auto knobs = col.reduced(0, 2);
        level.setBounds(knobs.removeFromLeft(oscColW).reduced(2), knobSize, labelH);
        pitch.setBounds(knobs.removeFromLeft(oscColW).reduced(2), knobSize, labelH);
        fine.setBounds(knobs.removeFromLeft(oscColW).reduced(2), knobSize, labelH);
        pan.setBounds(knobs.removeFromLeft(oscColW).reduced(2), knobSize, labelH);
        unison.setBounds(knobs.removeFromLeft(oscColW).reduced(2), knobSize, labelH);
        detune.setBounds(knobs.removeFromLeft(oscColW).reduced(2), knobSize, labelH);
        pw.setBounds(knobs.reduced(2), knobSize, labelH);
    };
    
    auto osc1Area = oscInner.removeFromLeft(oscColW); oscInner.removeFromLeft(6);
    layoutOsc(osc1Wave, osc1Level, osc1Pitch, osc1Fine, osc1Pan, osc1Unison, osc1Detune, osc1PW, osc1Area);
    
    auto osc2Area = oscInner.removeFromLeft(oscColW); oscInner.removeFromLeft(6);
    layoutOsc(osc2Wave, osc2Level, osc2Pitch, osc2Fine, osc2Pan, osc2Unison, osc2Detune, osc2PW, osc2Area);
    
    auto osc3Area = oscInner.removeFromLeft(oscColW); oscInner.removeFromLeft(6);
    layoutOsc(osc3Wave, osc3Level, osc3Pitch, osc3Fine, osc3Pan, osc3Unison, osc3Detune, osc3PW, osc3Area);
    
    // Sub + Noise
    auto subNoiseArea = oscInner; subNoiseArea.removeFromLeft(6);
    subWave.setBounds(subNoiseArea.removeFromTop(comboH).reduced(2));
    auto subKnobs = subNoiseArea.reduced(0, 4);
    subLevel.setBounds(subKnobs.removeFromLeft(subKnobs.getWidth()/2).reduced(2), knobSize, labelH);
    subPitch.setBounds(subKnobs.reduced(2), knobSize, labelH);
    
    subNoiseArea.removeFromTop(2);
    noiseWave.setBounds(subNoiseArea.removeFromTop(comboH).reduced(2));
    noiseLevel.setBounds(subNoiseArea.reduced(2, knobSize).reduced(2), knobSize, labelH);
    
    // Filter
    auto filtArea = leftCol.removeFromTop(170);
    filterPanel.setBounds(filtArea);
    auto filtInner = filtArea.reduced(14, 34);
    
    int fkw = (filtInner.getWidth() - 4 * 6) / 5;
    filterType.setBounds(filtInner.removeFromLeft(fkw + 20).removeFromTop(comboH).reduced(2));
    auto fknobs = filtInner;
    filterCutoff.setBounds(fknobs.removeFromLeft(fkw).reduced(2), knobSize, labelH);
    filterReso.setBounds(fknobs.removeFromLeft(fkw).reduced(2), knobSize, labelH);
    filterDrive.setBounds(fknobs.removeFromLeft(fkw).reduced(2), knobSize, labelH);
    filterKeyTrk.setBounds(fknobs.removeFromLeft(fkw).reduced(2), knobSize, labelH);
    filterVelTrk.setBounds(fknobs.reduced(2), knobSize, labelH);
    
    // === RIGHT: ENVELOPES + LFOS ===
    // Amp Env
    auto ampArea = rightCol.removeFromTop(150);
    ampEnvPanel.setBounds(ampArea);
    auto aInner = ampArea.reduced(14, 34);
    int akw = (aInner.getWidth() - 4 * 6) / 5;
    ampAtt.setBounds(aInner.removeFromLeft(akw).reduced(2), knobSize, labelH);
    ampDec.setBounds(aInner.removeFromLeft(akw).reduced(2), knobSize, labelH);
    ampSus.setBounds(aInner.removeFromLeft(akw).reduced(2), knobSize, labelH);
    ampRel.setBounds(aInner.removeFromLeft(akw).reduced(2), knobSize, labelH);
    ampVel.setBounds(aInner.reduced(2), knobSize, labelH);
    
    // Filt Env
    auto feArea = rightCol.removeFromTop(150);
    filtEnvPanel.setBounds(feArea);
    auto feInner = feArea.reduced(14, 34);
    int fekw = (feInner.getWidth() - 5 * 6) / 6;
    filtAtt.setBounds(feInner.removeFromLeft(fekw).reduced(2), knobSize, labelH);
    filtDec.setBounds(feInner.removeFromLeft(fekw).reduced(2), knobSize, labelH);
    filtSus.setBounds(feInner.removeFromLeft(fekw).reduced(2), knobSize, labelH);
    filtRel.setBounds(feInner.removeFromLeft(fekw).reduced(2), knobSize, labelH);
    filtAmt.setBounds(feInner.removeFromLeft(fekw).reduced(2), knobSize, labelH);
    filtVel.setBounds(feInner.reduced(2), knobSize, labelH);
    
    // LFOs
    auto lfoArea = rightCol.removeFromTop(180);
    lfoPanel.setBounds(lfoArea);
    auto lfoInner = lfoArea.reduced(14, 34);
    int lfoColW = lfoInner.getWidth() / 2;
    
    auto lfo1Area = lfoInner.removeFromLeft(lfoColW); lfoInner.removeFromLeft(6);
    lfo1Wave.setBounds(lfo1Area.removeFromTop(comboH).reduced(2));
    auto l1k = lfo1Area.reduced(0, 4);
    lfo1Rate.setBounds(l1k.removeFromLeft(70).reduced(2), knobSize, labelH);
    lfo1Amt.setBounds(l1k.removeFromLeft(70).reduced(2), knobSize, labelH);
    lfo1Delay.setBounds(l1k.removeFromLeft(70).reduced(2), knobSize, labelH);
    lfo1Fade.setBounds(l1k.reduced(2), knobSize, labelH);
    
    auto lfo2Area = lfoInner;
    lfo2Wave.setBounds(lfo2Area.removeFromTop(comboH).reduced(2));
    auto l2k = lfo2Area.reduced(0, 4);
    lfo2Rate.setBounds(l2k.removeFromLeft(70).reduced(2), knobSize, labelH);
    lfo2Amt.setBounds(l2k.removeFromLeft(70).reduced(2), knobSize, labelH);
    lfo2Delay.setBounds(l2k.removeFromLeft(70).reduced(2), knobSize, labelH);
    lfo2Fade.setBounds(l2k.reduced(2), knobSize, labelH);
    
    // Mod Matrix
    auto modArea = rightCol.removeFromTop(60);
    modPanel.setBounds(modArea);
    modLabel.setBounds(modArea.reduced(14, 0));
    modLabel.setText("MOD MATRIX — 8 slots (right-click knobs → Map to MIDI / Mod Matrix)", juce::dontSendNotification);
    modLabel.setJustificationType(juce::Justification::centred);
    modLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF666677));
    modLabel.setFont(11.0f);
}

// Knob instances
KnobGroup masterGainKnob, polyphonyKnob, pitchBendKnob;
KnobGroup osc1Level, osc1Pitch, osc1Fine, osc1Pan, osc1Unison, osc1Detune, osc1PW;
KnobGroup osc2Level, osc2Pitch, osc2Fine, osc2Pan, osc2Unison, osc2Detune, osc2PW;
KnobGroup osc3Level, osc3Pitch, osc3Fine, osc3Pan, osc3Unison, osc3Detune, osc3PW;
KnobGroup subLevel, subPitch, noiseLevel;
KnobGroup filterCutoff, filterReso, filterDrive, filterKeyTrk, filterVelTrk;
KnobGroup ampAtt, ampDec, ampSus, ampRel, ampVel;
KnobGroup filtAtt, filtDec, filtSus, filtRel, filtAmt, filtVel;
KnobGroup lfo1Rate, lfo1Amt, lfo1Delay, lfo1Fade;
KnobGroup lfo2Rate, lfo2Amt, lfo2Delay, lfo2Fade;

// Panels
SectionPanel oscPanel{"OSCILLATORS", juce::Colour(0xFF00D4AA)};
SectionPanel filterPanel{"FILTER", juce::Colour(0xFFFF8844)};
SectionPanel ampEnvPanel{"AMP ENVELOPE", juce::Colour(0xFF88AAFF)};
SectionPanel filtEnvPanel{"FILTER ENVELOPE", juce::Colour(0xFFAA88FF)};
SectionPanel lfoPanel{"LFOS", juce::Colour(0xFFFFAA88)};
SectionPanel modPanel{"MODULATION", juce::Colour(0xFF888888)};

// Wave combos
juce::ComboBox osc1Wave, osc2Wave, osc3Wave, subWave, noiseWave, filterType, lfo1Wave, lfo2Wave;
juce::ComboBox waveTypeComboBox;
juce::TextButton testToneButton;

// Labels
juce::Label modLabel;

// Attachments
std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> 
    osc1WaveAtt, osc2WaveAtt, osc3WaveAtt, subWaveAtt, noiseWaveAtt,
    filterTypeAtt, lfo1WaveAtt, lfo2WaveAtt, waveTypeAttachment;