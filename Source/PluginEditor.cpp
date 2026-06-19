#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
// SynthLookAndFeel
//==============================================================================
class AnalogSynthAudioProcessorEditor::SynthLookAndFeel : public juce::LookAndFeel_V4
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
        l->setFont(juce::FontOptions(11.0f));
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

//==============================================================================
// Helpers
//==============================================================================
static void styleCombo(juce::ComboBox& combo, juce::LookAndFeel* laf)
{
    combo.setLookAndFeel(laf);
}

static void setupCombo(juce::ComboBox& combo, const juce::String& paramID,
                        juce::AudioProcessorValueTreeState& apvts,
                        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>& attachment,
                        const juce::StringArray& choices, int defaultIdx,
                        juce::Component* parent, juce::LookAndFeel* laf)
{
    if (parent) parent->addAndMakeVisible(combo);
    styleCombo(combo, laf);
    for (int i = 0; i < choices.size(); ++i) combo.addItem(choices[i], i + 1);
    combo.setSelectedId(defaultIdx + 1);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, paramID, combo);
}

//==============================================================================
// SectionPanel implementation
//==============================================================================
AnalogSynthAudioProcessorEditor::SectionPanel::SectionPanel(const juce::String& title, juce::Colour accentColour)
    : titleText(title), accent(accentColour) {}

void AnalogSynthAudioProcessorEditor::SectionPanel::paint(juce::Graphics& g)
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
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.drawText(titleText, getLocalBounds().removeFromTop(28).reduced(12, 0),
               juce::Justification::centredLeft);
}

//==============================================================================
// KnobGroup implementation
//==============================================================================
void AnalogSynthAudioProcessorEditor::KnobGroup::setup(const juce::String& paramID, juce::AudioProcessorValueTreeState& apvts,
                  float min, float max, float interval, float def,
                  const juce::String& labelText, const juce::String& suffix,
                  juce::Component* parent, juce::LookAndFeel* laf)
{
    if (parent) parent->addAndMakeVisible(slider);
    slider.setLookAndFeel(laf);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 54, 20);
    slider.setTextValueSuffix(suffix);
    slider.setRange(min, max, interval);
    slider.setValue(def);

    if (parent) parent->addAndMakeVisible(label);
    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::FontOptions(10.0f));
    label.setColour(juce::Label::textColourId, juce::Colour(0xFF888888));

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID, slider);
}

void AnalogSynthAudioProcessorEditor::KnobGroup::setBounds(juce::Rectangle<int> area, int knobSize, int labelHeight)
{
    slider.setBounds(area.removeFromTop(knobSize).reduced(2));
    label.setBounds(area.removeFromTop(labelHeight).reduced(2));
}

//==============================================================================
// AnalogSynthAudioProcessorEditor
//==============================================================================
AnalogSynthAudioProcessorEditor::AnalogSynthAudioProcessorEditor (AnalogSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), apvts (p.getAPVTS())
{
    // Minimal constructor — defer all UI construction to avoid
    // Windows static-initialization / thread-init crashes.
    // The full UI is built on first resized() when the host's
    // GUI thread is fully ready.
    setSize (1100, 780);
    setResizable (true, true);
}

AnalogSynthAudioProcessorEditor::~AnalogSynthAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void AnalogSynthAudioProcessorEditor::buildUI()
{
    // Create look and feel
    lookAndFeel = std::make_unique<SynthLookAndFeel>();

    // === SECTION PANELS FIRST — so they're drawn UNDERNEATH the knobs ===
    oscPanel     = std::make_unique<SectionPanel>("OSCILLATORS",      juce::Colour(0xFF00D4AA));
    filterPanel  = std::make_unique<SectionPanel>("FILTER",           juce::Colour(0xFFFF8844));
    ampEnvPanel  = std::make_unique<SectionPanel>("AMP ENVELOPE",     juce::Colour(0xFF88AAFF));
    filtEnvPanel = std::make_unique<SectionPanel>("FILTER ENVELOPE",  juce::Colour(0xFFAA88FF));
    lfoPanel     = std::make_unique<SectionPanel>("LFOS",             juce::Colour(0xFFFFAA88));
    modPanel     = std::make_unique<SectionPanel>("MODULATION",       juce::Colour(0xFF888888));

    addAndMakeVisible(oscPanel.get());
    addAndMakeVisible(filterPanel.get());
    addAndMakeVisible(ampEnvPanel.get());
    addAndMakeVisible(filtEnvPanel.get());
    addAndMakeVisible(lfoPanel.get());
    addAndMakeVisible(modPanel.get());

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

    addAndMakeVisible(waveTypeComboBox);
    waveTypeComboBox.addItemList({"Sine", "Triangle", "Saw", "Square", "Noise"}, 1);
    waveTypeComboBox.setSelectedId(3);
    waveTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "osc1Wave", waveTypeComboBox);

    // Global knobs (header)
    masterGainKnob = std::make_unique<KnobGroup>(); masterGainKnob->setup("masterGain", apvts, 0.0f, 1.0f, 0.01f, 0.5f, "GAIN", " dB", this, lookAndFeel.get());
    polyphonyKnob = std::make_unique<KnobGroup>(); polyphonyKnob->setup("polyphony", apvts, 1.0f, 16.0f, 1.0f, 8.0f, "VOICES", "", this, lookAndFeel.get());
    pitchBendKnob = std::make_unique<KnobGroup>(); pitchBendKnob->setup("pitchBendRange", apvts, 0.0f, 24.0f, 0.5f, 2.0f, "PB RANGE", " st", this, lookAndFeel.get());

    // === OSCILLATORS ===
    setupCombo(osc1Wave, "osc1Wave", apvts, osc1WaveAtt, {"Sine", "Triangle", "Saw", "Square", "Noise"}, 2, this, lookAndFeel.get());
    osc1Level = std::make_unique<KnobGroup>(); osc1Level->setup("osc1Level", apvts, 0.0f, 1.0f, 0.01f, 0.7f, "LEVEL", "", this, lookAndFeel.get());
    osc1Pitch = std::make_unique<KnobGroup>(); osc1Pitch->setup("osc1Pitch", apvts, -24.0f, 24.0f, 1.0f, 0.0f, "PITCH", " st", this, lookAndFeel.get());
    osc1Fine = std::make_unique<KnobGroup>(); osc1Fine->setup("osc1FineTune", apvts, -50.0f, 50.0f, 1.0f, 0.0f, "FINE", " ct", this, lookAndFeel.get());
    osc1Pan = std::make_unique<KnobGroup>(); osc1Pan->setup("osc1Pan", apvts, -1.0f, 1.0f, 0.01f, 0.0f, "PAN", "", this, lookAndFeel.get());
    osc1Unison = std::make_unique<KnobGroup>(); osc1Unison->setup("osc1Unison", apvts, 1.0f, 8.0f, 1.0f, 1.0f, "UNISON", "", this, lookAndFeel.get());
    osc1Detune = std::make_unique<KnobGroup>(); osc1Detune->setup("osc1Detune", apvts, 0.0f, 50.0f, 1.0f, 0.0f, "DETUNE", " ct", this, lookAndFeel.get());
    osc1PW = std::make_unique<KnobGroup>(); osc1PW->setup("osc1PulseWidth", apvts, 0.01f, 0.99f, 0.01f, 0.5f, "PW", "%", this, lookAndFeel.get());

    setupCombo(osc2Wave, "osc2Wave", apvts, osc2WaveAtt, {"Sine", "Triangle", "Saw", "Square", "Noise"}, 2, this, lookAndFeel.get());
    osc2Level = std::make_unique<KnobGroup>(); osc2Level->setup("osc2Level", apvts, 0.0f, 1.0f, 0.01f, 0.5f, "LEVEL", "", this, lookAndFeel.get());
    osc2Pitch = std::make_unique<KnobGroup>(); osc2Pitch->setup("osc2Pitch", apvts, -24.0f, 24.0f, 1.0f, 0.0f, "PITCH", " st", this, lookAndFeel.get());
    osc2Fine = std::make_unique<KnobGroup>(); osc2Fine->setup("osc2FineTune", apvts, -50.0f, 50.0f, 1.0f, 0.0f, "FINE", " ct", this, lookAndFeel.get());
    osc2Pan = std::make_unique<KnobGroup>(); osc2Pan->setup("osc2Pan", apvts, -1.0f, 1.0f, 0.01f, 0.0f, "PAN", "", this, lookAndFeel.get());
    osc2Unison = std::make_unique<KnobGroup>(); osc2Unison->setup("osc2Unison", apvts, 1.0f, 8.0f, 1.0f, 1.0f, "UNISON", "", this, lookAndFeel.get());
    osc2Detune = std::make_unique<KnobGroup>(); osc2Detune->setup("osc2Detune", apvts, 0.0f, 50.0f, 1.0f, 0.0f, "DETUNE", " ct", this, lookAndFeel.get());
    osc2PW = std::make_unique<KnobGroup>(); osc2PW->setup("osc2PulseWidth", apvts, 0.01f, 0.99f, 0.01f, 0.5f, "PW", "%", this, lookAndFeel.get());

    setupCombo(osc3Wave, "osc3Wave", apvts, osc3WaveAtt, {"Sine", "Triangle", "Saw", "Square", "Noise"}, 2, this, lookAndFeel.get());
    osc3Level = std::make_unique<KnobGroup>(); osc3Level->setup("osc3Level", apvts, 0.0f, 1.0f, 0.01f, 0.3f, "LEVEL", "", this, lookAndFeel.get());
    osc3Pitch = std::make_unique<KnobGroup>(); osc3Pitch->setup("osc3Pitch", apvts, -24.0f, 24.0f, 1.0f, 0.0f, "PITCH", " st", this, lookAndFeel.get());
    osc3Fine = std::make_unique<KnobGroup>(); osc3Fine->setup("osc3FineTune", apvts, -50.0f, 50.0f, 1.0f, 0.0f, "FINE", " ct", this, lookAndFeel.get());
    osc3Pan = std::make_unique<KnobGroup>(); osc3Pan->setup("osc3Pan", apvts, -1.0f, 1.0f, 0.01f, 0.0f, "PAN", "", this, lookAndFeel.get());
    osc3Unison = std::make_unique<KnobGroup>(); osc3Unison->setup("osc3Unison", apvts, 1.0f, 8.0f, 1.0f, 1.0f, "UNISON", "", this, lookAndFeel.get());
    osc3Detune = std::make_unique<KnobGroup>(); osc3Detune->setup("osc3Detune", apvts, 0.0f, 50.0f, 1.0f, 0.0f, "DETUNE", " ct", this, lookAndFeel.get());
    osc3PW = std::make_unique<KnobGroup>(); osc3PW->setup("osc3PulseWidth", apvts, 0.01f, 0.99f, 0.01f, 0.5f, "PW", "%", this, lookAndFeel.get());

    setupCombo(subWave, "subWave", apvts, subWaveAtt, {"Sine", "Square"}, 0, this, lookAndFeel.get());
    subLevel = std::make_unique<KnobGroup>(); subLevel->setup("subLevel", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "SUB LEVEL", "", this, lookAndFeel.get());
    subPitch = std::make_unique<KnobGroup>(); subPitch->setup("subPitch", apvts, -24.0f, 0.0f, 1.0f, -12.0f, "SUB PITCH", " st", this, lookAndFeel.get());

    setupCombo(noiseWave, "noiseType", apvts, noiseWaveAtt, {"White", "Pink"}, 0, this, lookAndFeel.get());
    noiseLevel = std::make_unique<KnobGroup>(); noiseLevel->setup("noiseLevel", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "NOISE LEVEL", "", this, lookAndFeel.get());

    // === FILTER ===
    setupCombo(filterType, "filterType", apvts, filterTypeAtt, {"LP 4-Pole", "LP 2-Pole", "HP 4-Pole", "HP 2-Pole", "Bandpass", "Notch"}, 0, this, lookAndFeel.get());
    filterCutoff = std::make_unique<KnobGroup>(); filterCutoff->setup("filterCutoff", apvts, 20.0f, 20000.0f, 1.0f, 1000.0f, "CUTOFF", " Hz", this, lookAndFeel.get());
    filterReso = std::make_unique<KnobGroup>(); filterReso->setup("filterResonance", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "RESO", "", this, lookAndFeel.get());
    filterDrive = std::make_unique<KnobGroup>(); filterDrive->setup("filterDrive", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "DRIVE", "", this, lookAndFeel.get());
    filterKeyTrk = std::make_unique<KnobGroup>(); filterKeyTrk->setup("filterKeyTrack", apvts, -1.0f, 1.0f, 0.01f, 0.0f, "KEY TRK", "", this, lookAndFeel.get());
    filterVelTrk = std::make_unique<KnobGroup>(); filterVelTrk->setup("filterVelTrack", apvts, -1.0f, 1.0f, 0.01f, 0.0f, "VEL TRK", "", this, lookAndFeel.get());

    // === AMP ENV ===
    ampAtt = std::make_unique<KnobGroup>(); ampAtt->setup("ampAttack", apvts, 0.001f, 10.0f, 0.001f, 0.01f, "ATT", " s", this, lookAndFeel.get());
    ampDec = std::make_unique<KnobGroup>(); ampDec->setup("ampDecay", apvts, 0.001f, 10.0f, 0.001f, 0.3f, "DEC", " s", this, lookAndFeel.get());
    ampSus = std::make_unique<KnobGroup>(); ampSus->setup("ampSustain", apvts, 0.0f, 1.0f, 0.01f, 0.7f, "SUS", "", this, lookAndFeel.get());
    ampRel = std::make_unique<KnobGroup>(); ampRel->setup("ampRelease", apvts, 0.001f, 10.0f, 0.001f, 0.3f, "REL", " s", this, lookAndFeel.get());
    ampVel = std::make_unique<KnobGroup>(); ampVel->setup("ampVelSens", apvts, 0.0f, 1.0f, 0.01f, 0.5f, "VEL", "", this, lookAndFeel.get());

    // === FILT ENV ===
    filtAtt = std::make_unique<KnobGroup>(); filtAtt->setup("filtAttack", apvts, 0.001f, 10.0f, 0.001f, 0.01f, "ATT", " s", this, lookAndFeel.get());
    filtDec = std::make_unique<KnobGroup>(); filtDec->setup("filtDecay", apvts, 0.001f, 10.0f, 0.001f, 0.3f, "DEC", " s", this, lookAndFeel.get());
    filtSus = std::make_unique<KnobGroup>(); filtSus->setup("filtSustain", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "SUS", "", this, lookAndFeel.get());
    filtRel = std::make_unique<KnobGroup>(); filtRel->setup("filtRelease", apvts, 0.001f, 10.0f, 0.001f, 0.3f, "REL", " s", this, lookAndFeel.get());
    filtAmt = std::make_unique<KnobGroup>(); filtAmt->setup("filtAmount", apvts, -1.0f, 1.0f, 0.01f, 0.5f, "AMT", "", this, lookAndFeel.get());
    filtVel = std::make_unique<KnobGroup>(); filtVel->setup("filtVelSens", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "VEL", "", this, lookAndFeel.get());

    // === LFOS ===
    setupCombo(lfo1Wave, "lfo1Wave", apvts, lfo1WaveAtt, {"Sine", "Triangle", "Saw", "Square", "S&H"}, 0, this, lookAndFeel.get());
    lfo1Rate = std::make_unique<KnobGroup>(); lfo1Rate->setup("lfo1Rate", apvts, 0.01f, 20.0f, 0.01f, 1.0f, "RATE", " Hz", this, lookAndFeel.get());
    lfo1Amt = std::make_unique<KnobGroup>(); lfo1Amt->setup("lfo1Amount", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "AMT", "", this, lookAndFeel.get());
    lfo1Delay = std::make_unique<KnobGroup>(); lfo1Delay->setup("lfo1Delay", apvts, 0.0f, 5.0f, 0.01f, 0.0f, "DELAY", " s", this, lookAndFeel.get());
    lfo1Fade = std::make_unique<KnobGroup>(); lfo1Fade->setup("lfo1Fade", apvts, 0.0f, 5.0f, 0.01f, 0.0f, "FADE", " s", this, lookAndFeel.get());

    setupCombo(lfo2Wave, "lfo2Wave", apvts, lfo2WaveAtt, {"Sine", "Triangle", "Saw", "Square", "S&H"}, 1, this, lookAndFeel.get());
    lfo2Rate = std::make_unique<KnobGroup>(); lfo2Rate->setup("lfo2Rate", apvts, 0.01f, 20.0f, 0.01f, 5.0f, "RATE", " Hz", this, lookAndFeel.get());
    lfo2Amt = std::make_unique<KnobGroup>(); lfo2Amt->setup("lfo2Amount", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "AMT", "", this, lookAndFeel.get());
    lfo2Delay = std::make_unique<KnobGroup>(); lfo2Delay->setup("lfo2Delay", apvts, 0.0f, 5.0f, 0.01f, 0.0f, "DELAY", " s", this, lookAndFeel.get());
    lfo2Fade = std::make_unique<KnobGroup>(); lfo2Fade->setup("lfo2Fade", apvts, 0.0f, 5.0f, 0.01f, 0.0f, "FADE", " s", this, lookAndFeel.get());

    // Mod label
    addAndMakeVisible(modLabel);
    modLabel.setText("MOD MATRIX — 8 slots (right-click knobs → Map to MIDI / Mod Matrix)", juce::dontSendNotification);
    modLabel.setJustificationType(juce::Justification::centred);
    modLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF666677));
    modLabel.setFont(juce::FontOptions(11.0f));
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
    g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    g.drawText("AnalogSynth", header.removeFromLeft(180).reduced(16, 0), juce::Justification::centredLeft);

    g.setFont(juce::FontOptions(11.0f));
    g.setColour(juce::Colour(0xFF666677));
    auto statusArea = header.removeFromRight(300);
    g.drawText(processorRef.isTestToneActive() ? "● TEST ACTIVE" : "○ TEST READY",
               statusArea.reduced(16, 0), juce::Justification::centredRight);
}

void AnalogSynthAudioProcessorEditor::resized()
{
    // Deferred UI construction — safe to build now on the message thread
    if (!initialized)
    {
        buildUI();
        initialized = true;
    }

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
    masterGainKnob->slider.setBounds(globalArea.removeFromLeft(70).reduced(2));
    masterGainKnob->label.setBounds(globalArea.removeFromLeft(70).reduced(2).withHeight(labelH).withY(masterGainKnob->slider.getBottom()));
    polyphonyKnob->slider.setBounds(globalArea.removeFromLeft(70).reduced(2));
    polyphonyKnob->label.setBounds(globalArea.removeFromLeft(70).reduced(2).withHeight(labelH).withY(polyphonyKnob->slider.getBottom()));
    pitchBendKnob->slider.setBounds(globalArea.removeFromLeft(70).reduced(2));
    pitchBendKnob->label.setBounds(globalArea.removeFromLeft(70).reduced(2).withHeight(labelH).withY(pitchBendKnob->slider.getBottom()));

    bounds.removeFromTop(sectionGap);

    // 2-column layout
    auto leftCol = bounds.removeFromLeft((bounds.getWidth() * 3) / 5);
    auto rightCol = bounds;

    // === LEFT: OSC + FILTER ===
    auto oscArea = leftCol.removeFromTop(340);
    oscPanel->setBounds(oscArea);
    auto oscInner = oscArea.reduced(14, 34);

    int oscColW = (oscInner.getWidth() - 4 * 6) / 5;

    auto layoutOsc = [&](juce::ComboBox& wave, KnobGroup* level, KnobGroup* pitch, KnobGroup* fine,
                         KnobGroup* pan, KnobGroup* unison, KnobGroup* detune, KnobGroup* pw,
                         juce::Rectangle<int> col)
    {
        wave.setBounds(col.removeFromTop(comboH).reduced(2));
        auto knobs = col.reduced(0, 2);
        level->setBounds(knobs.removeFromLeft(oscColW).reduced(2), knobSize, labelH);
        pitch->setBounds(knobs.removeFromLeft(oscColW).reduced(2), knobSize, labelH);
        fine->setBounds(knobs.removeFromLeft(oscColW).reduced(2), knobSize, labelH);
        pan->setBounds(knobs.removeFromLeft(oscColW).reduced(2), knobSize, labelH);
        unison->setBounds(knobs.removeFromLeft(oscColW).reduced(2), knobSize, labelH);
        detune->setBounds(knobs.removeFromLeft(oscColW).reduced(2), knobSize, labelH);
        pw->setBounds(knobs.reduced(2), knobSize, labelH);
    };

    auto osc1Area = oscInner.removeFromLeft(oscColW); oscInner.removeFromLeft(6);
    layoutOsc(osc1Wave, osc1Level.get(), osc1Pitch.get(), osc1Fine.get(), osc1Pan.get(), osc1Unison.get(), osc1Detune.get(), osc1PW.get(), osc1Area);

    auto osc2Area = oscInner.removeFromLeft(oscColW); oscInner.removeFromLeft(6);
    layoutOsc(osc2Wave, osc2Level.get(), osc2Pitch.get(), osc2Fine.get(), osc2Pan.get(), osc2Unison.get(), osc2Detune.get(), osc2PW.get(), osc2Area);

    auto osc3Area = oscInner.removeFromLeft(oscColW); oscInner.removeFromLeft(6);
    layoutOsc(osc3Wave, osc3Level.get(), osc3Pitch.get(), osc3Fine.get(), osc3Pan.get(), osc3Unison.get(), osc3Detune.get(), osc3PW.get(), osc3Area);

    // Sub + Noise
    auto subNoiseArea = oscInner; subNoiseArea.removeFromLeft(6);
    subWave.setBounds(subNoiseArea.removeFromTop(comboH).reduced(2));
    auto subKnobs = subNoiseArea.reduced(0, 4);
    subLevel->setBounds(subKnobs.removeFromLeft(subKnobs.getWidth()/2).reduced(2), knobSize, labelH);
    subPitch->setBounds(subKnobs.reduced(2), knobSize, labelH);

    subNoiseArea.removeFromTop(2);
    noiseWave.setBounds(subNoiseArea.removeFromTop(comboH).reduced(2));
    noiseLevel->setBounds(subNoiseArea.reduced(2, knobSize).reduced(2), knobSize, labelH);

    // Filter
    auto filtArea = leftCol.removeFromTop(170);
    filterPanel->setBounds(filtArea);
    auto filtInner = filtArea.reduced(14, 34);

    int fkw = (filtInner.getWidth() - 4 * 6) / 5;
    filterType.setBounds(filtInner.removeFromLeft(fkw + 20).removeFromTop(comboH).reduced(2));
    auto fknobs = filtInner;
    filterCutoff->setBounds(fknobs.removeFromLeft(fkw).reduced(2), knobSize, labelH);
    filterReso->setBounds(fknobs.removeFromLeft(fkw).reduced(2), knobSize, labelH);
    filterDrive->setBounds(fknobs.removeFromLeft(fkw).reduced(2), knobSize, labelH);
    filterKeyTrk->setBounds(fknobs.removeFromLeft(fkw).reduced(2), knobSize, labelH);
    filterVelTrk->setBounds(fknobs.reduced(2), knobSize, labelH);

    // === RIGHT: ENVELOPES + LFOS ===
    // Amp Env
    auto ampArea = rightCol.removeFromTop(150);
    ampEnvPanel->setBounds(ampArea);
    auto aInner = ampArea.reduced(14, 34);
    int akw = (aInner.getWidth() - 4 * 6) / 5;
    ampAtt->setBounds(aInner.removeFromLeft(akw).reduced(2), knobSize, labelH);
    ampDec->setBounds(aInner.removeFromLeft(akw).reduced(2), knobSize, labelH);
    ampSus->setBounds(aInner.removeFromLeft(akw).reduced(2), knobSize, labelH);
    ampRel->setBounds(aInner.removeFromLeft(akw).reduced(2), knobSize, labelH);
    ampVel->setBounds(aInner.reduced(2), knobSize, labelH);

    // Filt Env
    auto feArea = rightCol.removeFromTop(150);
    filtEnvPanel->setBounds(feArea);
    auto feInner = feArea.reduced(14, 34);
    int fekw = (feInner.getWidth() - 5 * 6) / 6;
    filtAtt->setBounds(feInner.removeFromLeft(fekw).reduced(2), knobSize, labelH);
    filtDec->setBounds(feInner.removeFromLeft(fekw).reduced(2), knobSize, labelH);
    filtSus->setBounds(feInner.removeFromLeft(fekw).reduced(2), knobSize, labelH);
    filtRel->setBounds(feInner.removeFromLeft(fekw).reduced(2), knobSize, labelH);
    filtAmt->setBounds(feInner.removeFromLeft(fekw).reduced(2), knobSize, labelH);
    filtVel->setBounds(feInner.reduced(2), knobSize, labelH);

    // LFOs
    auto lfoArea = rightCol.removeFromTop(180);
    lfoPanel->setBounds(lfoArea);
    auto lfoInner = lfoArea.reduced(14, 34);
    int lfoColW = lfoInner.getWidth() / 2;

    auto lfo1Area = lfoInner.removeFromLeft(lfoColW); lfoInner.removeFromLeft(6);
    lfo1Wave.setBounds(lfo1Area.removeFromTop(comboH).reduced(2));
    auto l1k = lfo1Area.reduced(0, 4);
    lfo1Rate->setBounds(l1k.removeFromLeft(70).reduced(2), knobSize, labelH);
    lfo1Amt->setBounds(l1k.removeFromLeft(70).reduced(2), knobSize, labelH);
    lfo1Delay->setBounds(l1k.removeFromLeft(70).reduced(2), knobSize, labelH);
    lfo1Fade->setBounds(l1k.reduced(2), knobSize, labelH);

    auto lfo2Area = lfoInner;
    lfo2Wave.setBounds(lfo2Area.removeFromTop(comboH).reduced(2));
    auto l2k = lfo2Area.reduced(0, 4);
    lfo2Rate->setBounds(l2k.removeFromLeft(70).reduced(2), knobSize, labelH);
    lfo2Amt->setBounds(l2k.removeFromLeft(70).reduced(2), knobSize, labelH);
    lfo2Delay->setBounds(l2k.removeFromLeft(70).reduced(2), knobSize, labelH);
    lfo2Fade->setBounds(l2k.reduced(2), knobSize, labelH);

    // Mod Matrix
    auto modArea = rightCol.removeFromTop(60);
    modPanel->setBounds(modArea);
    modLabel.setBounds(modArea.reduced(14, 0));
}
