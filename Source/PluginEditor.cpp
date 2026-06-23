#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
// SynthLookAndFeel — premium dark theme v3
//   • 48px rotary knobs with glow arc, indicator line, tick marks
//   • Surge-style metallic bezel + dark inner face
//   • Cyan/amber-gold/purple accent system
//   • Monospace numeric readouts
//   • Pill toggle buttons
//==============================================================================
class AnalogSynthAudioProcessorEditor::SynthLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SynthLookAndFeel()
    {
        setColour (juce::Slider::rotarySliderFillColourId,    juce::Colour(ColourID::kSignalPath));
        setColour (juce::Slider::rotarySliderOutlineColourId,  juce::Colour(0xFF1C1C30));
        setColour (juce::Slider::thumbColourId,                juce::Colours::white);
        setColour (juce::ComboBox::backgroundColourId,         juce::Colour(0xFF0E0E1A));
        setColour (juce::ComboBox::outlineColourId,            juce::Colour(0xFF2A2A40));
        setColour (juce::ComboBox::textColourId,               juce::Colour(ColourID::kTextPrimary));
        setColour (juce::ComboBox::arrowColourId,              juce::Colour(ColourID::kTextMuted));
        setColour (juce::Label::textColourId,                  juce::Colour(ColourID::kTextMuted));
        setColour (juce::PopupMenu::backgroundColourId,        juce::Colour(0xFF12121E));
        setColour (juce::PopupMenu::textColourId,              juce::Colour(ColourID::kTextPrimary));
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(ColourID::kSignalPath).withAlpha(0.2f));
        setColour (juce::PopupMenu::highlightedTextColourId,   juce::Colours::white);
        setColour (juce::TextButton::buttonColourId,           juce::Colour(0xFF151528));
        setColour (juce::TextButton::buttonOnColourId,         juce::Colour(ColourID::kSignalPath));
        setColour (juce::TextButton::textColourOffId,          juce::Colour(ColourID::kTextDim));
        setColour (juce::TextButton::textColourOnId,           juce::Colours::black);
    }

    // ── Rotary knob — Surge-style bezel + arc + indicator line ──────
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                           float pos, float start, float end, juce::Slider& s) override
    {
        const auto r      = juce::Rectangle<float>((float)x, (float)y, (float)w, (float)h).reduced(5);
        const auto rad    = juce::jmin(r.getWidth(), r.getHeight()) / 2.0f;
        const auto c      = r.getCentre();
        const auto accent = s.findColour(juce::Slider::rotarySliderFillColourId);
        const float angle = start + pos * (end - start);

        // Metallic bezel ring
        juce::ColourGradient bezel(
            juce::Colour(0xFF353548), c.x - rad, c.y - rad,
            juce::Colour(0xFF181828), c.x + rad, c.y + rad, false);
        g.setGradientFill(bezel);
        g.fillEllipse(c.x - rad, c.y - rad, rad * 2, rad * 2);
        g.setColour(juce::Colour(0xFF101020));
        g.drawEllipse(c.x - rad + 0.5f, c.y - rad + 0.5f, rad * 2 - 1, rad * 2 - 1, 0.7f);

        // Dark inner face
        const float innerR = rad - 3.5f;
        g.setColour(juce::Colour(0xFF0A0A14));
        g.fillEllipse(c.x - innerR, c.y - innerR, innerR * 2, innerR * 2);
        g.setColour(juce::Colour(0xFF1C1C2C));
        g.drawEllipse(c.x - innerR, c.y - innerR, innerR * 2, innerR * 2, 0.5f);

        // Tick marks (25 marks around sweep, every 5th major)
        for (int i = 0; i <= 25; ++i)
        {
            const float ta = start + (end - start) * i / 25.0f;
            const bool major = (i % 5 == 0);
            const auto p1 = c.getPointOnCircumference(innerR - 2.5f, ta);
            const auto p2 = c.getPointOnCircumference(innerR - (major ? 0.0f : 1.5f), ta);
            g.setColour(juce::Colour(0xFF383850).withAlpha(major ? 0.7f : 0.3f));
            g.drawLine(p1.x, p1.y, p2.x, p2.y, major ? 1.0f : 0.5f);
        }

        // Glow arc behind value
        juce::Path glowArc;
        glowArc.addCentredArc(c.x, c.y, innerR - 3, innerR - 3, 0, start, angle, true);
        g.setColour(accent.withAlpha(0.10f));
        g.strokePath(glowArc, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Value arc
        juce::Path valueArc;
        valueArc.addCentredArc(c.x, c.y, innerR - 3, innerR - 3, 0, start, angle, true);
        g.setColour(accent.withAlpha(0.88f));
        g.strokePath(valueArc, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Indicator line (centre → edge)
        const auto indOuter = c.getPointOnCircumference(innerR - 4, angle);
        const auto indInner = c.getPointOnCircumference(innerR * 0.25f, angle);
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.drawLine(indInner.x, indInner.y, indOuter.x, indOuter.y, 1.3f);

        // Centre cap dot
        g.setColour(juce::Colour(0xFF0A0A14));
        g.fillEllipse(c.x - 3, c.y - 3, 6, 6);
        g.setColour(juce::Colour(0xFF1E1E30));
        g.drawEllipse(c.x - 3, c.y - 3, 6, 6, 0.4f);
    }

    // ── Text box ────────────────────────────────────────────────────
    juce::Label* createSliderTextBox(juce::Slider& s) override
    {
        auto* l = LookAndFeel_V4::createSliderTextBox(s);
        l->setColour(juce::Label::outlineColourId,    juce::Colours::transparentBlack);
        l->setColour(juce::Label::backgroundColourId, juce::Colour(0xFF0A0A12));
        l->setColour(juce::Label::textColourId,       juce::Colour(ColourID::kTextPrimary));
        l->setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::plain));
        l->setJustificationType(juce::Justification::centred);
        return l;
    }

    // ── Combo box — inset dark style ────────────────────────────────
    void drawComboBox(juce::Graphics& g, int w, int h, bool,
                       int bx, int by, int bw, int bh, juce::ComboBox& box) override
    {
        auto r = juce::Rectangle<int>(0, 0, w, h);
        g.setColour(findColour(juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle(r.toFloat(), 4.0f);
        g.setColour(findColour(juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle(r.toFloat().reduced(0.5f), 4.0f, 0.8f);
        g.setColour(juce::Colour(0xFF202035).withAlpha(0.4f));
        g.drawRoundedRectangle(r.toFloat().reduced(1.5f), 3.0f, 0.5f);

        juce::Path arrow;
        arrow.addTriangle(0, 0, 7, 0, 3.5f, 4);
        g.setColour(findColour(juce::ComboBox::arrowColourId));
        g.fillPath(arrow, juce::AffineTransform::translation((float)(w - 15), (float)((h - 3) / 2)));
    }

    // ── Text button — pill toggle ───────────────────────────────────
    void drawButtonBackground(juce::Graphics& g, juce::Button& btn,
                              const juce::Colour& bg, bool isOver, bool isDown) override
    {
        auto b = btn.getLocalBounds().toFloat().reduced(1);
        auto col = btn.getToggleState() ? btn.findColour(juce::TextButton::buttonOnColourId)
                                        : btn.findColour(juce::TextButton::buttonColourId);
        if (isOver) col = col.brighter(0.12f);
        if (isDown) col = col.darker(0.08f);
        g.setColour(col);
        g.fillRoundedRectangle(b, 5.0f);
        if (!btn.getToggleState())
        {
            g.setColour(juce::Colour(0xFF2A2A3A));
            g.drawRoundedRectangle(b, 5.0f, 0.6f);
        }
    }
};

//==============================================================================
// SectionPanel — Surge-style rack module with layered depth
//==============================================================================
AnalogSynthAudioProcessorEditor::SectionPanel::SectionPanel (const juce::String& t, juce::Colour a)
    : title (t), accent (a) {}

void AnalogSynthAudioProcessorEditor::SectionPanel::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    // --- Outer shadow rim ---
    g.setColour(juce::Colour(0xFF050508));
    g.fillRoundedRectangle(b, 8.0f);

    // --- Card body — dark gradient ---
    auto body = b.reduced(1.0f);
    juce::ColourGradient bodyGrad(
        juce::Colour(0xFF141422), body.getTopLeft(),
        juce::Colour(0xFF0C0C18), body.getBottomLeft(), false);
    g.setGradientFill(bodyGrad);
    g.fillRoundedRectangle(body, 7.5f);

    // --- Subtle inner highlight at top ---
    g.setColour(juce::Colour(0xFF1C1C2E).withAlpha(0.6f));
    g.drawLine(body.getX() + 12, body.getY() + 2, body.getRight() - 12, body.getY() + 2, 0.5f);

    // --- Header background with accent gradient ---
    auto hdr = body.withHeight(32);
    juce::ColourGradient hdrGrad(
        accent.withAlpha(0.18f), hdr.getTopLeft(),
        accent.withAlpha(0.03f), hdr.getBottomLeft(), false);
    g.setGradientFill(hdrGrad);
    g.fillRoundedRectangle(hdr.getX(), hdr.getY(), hdr.getWidth(), hdr.getHeight(), 7.5f);
    g.fillRect(hdr.withTrimmedTop(24));  // extend below rounded top

    // --- Accent line under header ---
    g.setColour(accent.withAlpha(0.55f));
    g.drawLine(hdr.getX() + 14, hdr.getBottom(), hdr.getRight() - 14, hdr.getBottom(), 1.2f);

    // --- Title text ---
    g.setColour(accent.withAlpha(0.95f));
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::bold));
    g.drawText(title, hdr.getX() + 14, hdr.getY(), hdr.getWidth() - 28, hdr.getHeight(),
               juce::Justification::centredLeft);

    // --- Card border ---
    g.setColour(juce::Colour(0xFF222236).withAlpha(0.5f));
    g.drawRoundedRectangle(body, 7.5f, 0.6f);
}

//==============================================================================
// WaveformPreview
//==============================================================================
void AnalogSynthAudioProcessorEditor::WaveformPreview::paint (juce::Graphics& g)
{
    auto b    = getLocalBounds().toFloat();
    auto w    = b.getWidth();
    auto h    = b.getHeight();
    auto mid  = h / 2.0f;
    auto amp  = h * 0.35f;

    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillRoundedRectangle (b, 3.0f);

    juce::Path p;
    p.startNewSubPath (0, mid);

    if (waveType == "Sine") {
        for (float x = 0; x <= w; x += 1.0f)
            p.lineTo (x, mid - std::sin (x / w * juce::MathConstants<float>::twoPi * 2.0f) * amp);
    } else if (waveType == "Triangle") {
        for (float x = 0; x <= w; x += 1.0f) {
            float ph = std::fmod (x / w, 1.0f);
            p.lineTo (x, mid - (4.0f * std::fabs (ph - 0.5f) - 1.0f) * amp);
        }
    } else if (waveType == "Saw") {
        for (float x = 0; x <= w; x += 1.0f) {
            float ph = std::fmod (x / w, 1.0f);
            p.lineTo (x, mid - (ph * 2.0f - 1.0f) * amp);
        }
    } else if (waveType == "Square") {
        for (float x = 0; x <= w; x += 1.0f)
            p.lineTo (x, mid - (std::fmod (x / w, 1.0f) < 0.5f ? 1.0f : -1.0f) * amp);
    } else if (waveType == "Noise") {
        juce::Random rng;
        for (float x = 0; x <= w; x += 1.0f)
            p.lineTo (x, mid + (rng.nextFloat() - 0.5f) * amp * 2.0f);
    } else if (waveType == "Wavetable") {
        for (float x = 0; x <= w; x += 1.0f) {
            float ph = x / w;
            float val = std::sin (ph * juce::MathConstants<float>::twoPi * 3.0f) * 0.5f
                      + std::sin (ph * juce::MathConstants<float>::twoPi * 7.0f) * 0.3f
                      + std::sin (ph * juce::MathConstants<float>::twoPi * 12.0f) * 0.2f;
            p.lineTo (x, mid - val * amp);
        }
    }
    g.setColour (juce::Colour(ColourID::kSignalPath).withAlpha (0.7f));
    g.strokePath (p, juce::PathStrokeType (1.0f));
}

//==============================================================================
// EnvDisplay
//==============================================================================
void AnalogSynthAudioProcessorEditor::EnvDisplay::setParams (float att, float dec, float sus, float rel)
{
    attack  = att;
    decay   = dec;
    sustain = sus;
    release = rel;
    repaint();
}

void AnalogSynthAudioProcessorEditor::EnvDisplay::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced(2);

    g.setColour (juce::Colour(0xFF111118));
    g.fillRoundedRectangle (b, 4.0f);
    g.setColour (juce::Colour(0xFF2A2A35));
    g.drawRoundedRectangle (b, 4.0f, 0.5f);

    juce::Path p;
    float total = attack + decay + release + 0.02f;
    if (total < 0.001f) return;

    float sx = b.getX(), sy = b.getBottom();
    p.startNewSubPath (sx, sy);

    float x1 = sx + (attack / total) * b.getWidth();
    p.lineTo (x1, b.getY() + 4);
    float x2 = x1 + (decay / total) * b.getWidth();
    float susY = b.getY() + (1.0f - sustain) * (b.getHeight() - 8);
    p.lineTo (x2, susY);
    float x3 = x2 + (0.02f / total) * b.getWidth();
    p.lineTo (x3, susY);
    float x4 = x3 + (release / total) * b.getWidth();
    p.lineTo (x4, sy);

    g.setColour (juce::Colour(0xFF00E5B0).withAlpha(0.85f));
    g.strokePath (p, juce::PathStrokeType(1.8f));

    auto dot = [&](float x, float y) {
        g.setColour (juce::Colour(0xFF00E5B0).withAlpha(0.9f));
        g.fillEllipse (x - 3, y - 3, 6, 6);
    };
    dot (x1, b.getY() + 4);
    dot (x2, susY);
}

//==============================================================================
// FilterCurveDisplay
//==============================================================================
void AnalogSynthAudioProcessorEditor::FilterCurveDisplay::setParams (int type, float cutoff, float reso)
{
    filterType  = type;
    cutoffFreq  = cutoff;
    resonance   = reso;
    repaint();
}

void AnalogSynthAudioProcessorEditor::FilterCurveDisplay::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced(2);

    // dark background
    g.setColour (juce::Colour(0xFF111118));
    g.fillRoundedRectangle (b, 4.0f);
    g.setColour (juce::Colour(0xFF2A2A35));
    g.drawRoundedRectangle (b, 4.0f, 0.5f);

    // grid lines
    g.setColour (juce::Colour(0xFF1A1A26));
    for (int i = 1; i < 4; ++i) {
        float y = b.getY() + b.getHeight() * i / 4.0f;
        g.drawLine (b.getX(), y, b.getRight(), y, 0.5f);
    }
    for (int i = 1; i < 4; ++i) {
        float x = b.getX() + b.getWidth() * i / 4.0f;
        g.drawLine (x, b.getY(), x, b.getBottom(), 0.5f);
    }

    // draw filter response curve
    juce::Path p;
    p.startNewSubPath (b.getX(), b.getBottom());

    float normCutoff = std::log(std::max(20.0f, cutoffFreq) / 20.0f) / std::log(20000.0f / 20.0f);
    float cx = b.getX() + normCutoff * b.getWidth();

    for (float x = b.getX(); x <= b.getRight(); x += 1.0f)
    {
        float freq = 20.0f * std::pow(20000.0f / 20.0f, (x - b.getX()) / b.getWidth());
        float ratio = freq / std::max(20.0f, cutoffFreq);
        int poles = (filterType <= 1) ? 4 : (filterType <= 3) ? 2 : 1;
        float gain = 1.0f / std::sqrt(1.0f + std::pow(ratio, 2.0f * poles));

        // resonance peak
        if (resonance > 0.01f && freq > cutoffFreq * 0.3f && freq < cutoffFreq * 3.0f) {
            float dist = std::abs(freq - cutoffFreq) / cutoffFreq;
            float q = resonance + 0.1f;
            float peak = resonance * std::exp(-dist * dist * 30.0f / q);
            gain += peak * 0.6f;
        }

        float db = juce::Decibels::gainToDecibels(gain, -48.0f);
        float y = b.getBottom() - (db + 48.0f) / 48.0f * b.getHeight();
        p.lineTo (x, juce::jlimit(b.getY(), b.getBottom(), y));
    }

    g.setColour (juce::Colour(ColourID::kSignalPath).withAlpha(0.8f));
    g.strokePath (p, juce::PathStrokeType(1.5f));

    // cutoff marker
    g.setColour (juce::Colour(ColourID::kSignalPath).withAlpha(0.25f));
    g.drawLine (cx, b.getY() + 4, cx, b.getBottom() - 4, 0.5f);
}

//==============================================================================
// SignalFlowDiagram — visual routing between modules
//   OSC → MIX → FLT → AMP → FX
//==============================================================================
void AnalogSynthAudioProcessorEditor::SignalFlowDiagram::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced(4, 2);
    auto w = b.getWidth();
    auto h = b.getHeight();
    auto midY = b.getCentreY();

    // Background track
    g.setColour(juce::Colour(ColourID::kSurface));
    g.fillRoundedRectangle(b, 6.0f);

    // Module boxes — evenly spaced
    const char* labels[] = {"OSC", "MIX", "FLT", "AMP", "FX"};
    const juce::Colour colours[] = {
        juce::Colour(ColourID::kSignalPath),
        juce::Colour(ColourID::kSignalPath).withAlpha(0.7f),
        juce::Colour(ColourID::kSignalPath),
        juce::Colour(ColourID::kModulation),
        juce::Colour(ColourID::kEffects)
    };
    constexpr int numBlocks = 5;
    float blockW = 52;
    float totalW = numBlocks * blockW + (numBlocks - 1) * 18;
    float startX = b.getX() + (w - totalW) / 2.0f;

    for (int i = 0; i < numBlocks; ++i)
    {
        float bx = startX + i * (blockW + 18);
        auto block = juce::Rectangle<float>(bx, midY - 14, blockW, 28);

        // Block background
        g.setColour(colours[i].withAlpha(0.18f));
        g.fillRoundedRectangle(block, 5.0f);
        g.setColour(colours[i].withAlpha(0.5f));
        g.drawRoundedRectangle(block, 5.0f, 1.0f);

        // Label
        g.setColour(colours[i].withAlpha(0.9f));
        g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::bold));
        g.drawText(labels[i], block, juce::Justification::centred);

        // Arrow connector (except last)
        if (i < numBlocks - 1)
        {
            float arrowStart = block.getRight();
            float arrowEnd   = arrowStart + 18;
            g.setColour(colours[i].withAlpha(0.35f));
            g.drawLine(arrowStart + 2, midY, arrowEnd - 2, midY, 1.0f);
            // Arrowhead
            juce::Path arrow;
            arrow.addTriangle(arrowEnd - 2, midY - 4,
                              arrowEnd - 2, midY + 4,
                              arrowEnd,     midY);
            g.fillPath(arrow);
        }
    }
}

//==============================================================================
// KnobGroup
//==============================================================================
void AnalogSynthAudioProcessorEditor::KnobGroup::setup (
    const juce::String& id, juce::AudioProcessorValueTreeState& a,
    float min, float max, float step, float def,
    const juce::String& txt, const juce::String& suffix,
    juce::Component* parent, juce::LookAndFeel* lf,
    juce::Colour accent)
{
    if (parent)
    {
        parent->addAndMakeVisible (slider);
        parent->addAndMakeVisible (label);
    }

    slider.setLookAndFeel (lf);
    slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 52, 20);
    slider.setTextValueSuffix (suffix);
    slider.setRange (min, max, step);
    slider.setValue (def);

    label.setText (txt, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::FontOptions (9.0f));
    label.setColour (juce::Label::textColourId, juce::Colour (0xFF777788));

    att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (a, id, slider);
}

//==============================================================================
// Helpers
//==============================================================================
static void setCombo (juce::ComboBox& cb, const juce::String& id,
                      juce::AudioProcessorValueTreeState& a,
                      std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>& att,
                      const juce::StringArray& choices, int defIdx,
                      juce::Component* parent, juce::LookAndFeel* lf)
{
    if (parent) parent->addAndMakeVisible (cb);
    cb.setLookAndFeel (lf);
    for (int i = 0; i < choices.size(); ++i)
        cb.addItem (choices[i], i + 1);
    cb.setSelectedId (defIdx + 1, juce::dontSendNotification);
    att = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (a, id, cb);
}

//==============================================================================
// Editor
//==============================================================================
AnalogSynthAudioProcessorEditor::AnalogSynthAudioProcessorEditor (AnalogSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p), apvts (p.getAPVTS())
{
    setSize (1050, 780);
    setResizable (true, true);
}

AnalogSynthAudioProcessorEditor::~AnalogSynthAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void AnalogSynthAudioProcessorEditor::buildUI()
{
    laf = std::make_unique<SynthLookAndFeel>();

    // Signal flow diagram (top of content)
    addAndMakeVisible(signalFlow);

    // panels
    addAndMakeVisible (oscPanel);
    addAndMakeVisible (filtPanel);
    addAndMakeVisible (ampPanel);
    addAndMakeVisible (fenvPanel);
    addAndMakeVisible (arpPanel);
    addAndMakeVisible (lfoPanel);
    addAndMakeVisible (fxPanel);
    addAndMakeVisible (macroPanel);
    addAndMakeVisible (modPanel);

    // Global
    // Preset selector
    addAndMakeVisible(presetBox);
    initPresets();
    masterGain.setup ("masterGain",      apvts, 0.0f, 1.0f, 0.01f, 0.5f,  "GAIN",  " dB", this, laf.get());
    polyphony.setup ("polyphony",        apvts, 1.0f, 16.0f, 1.0f, 8.0f,  "VOICES","",    this, laf.get());
    pitchBend.setup  ("pitchBendRange",  apvts, 0.0f, 24.0f, 0.5f, 2.0f,  "BEND",  " st", this, laf.get());

    // --- Osc 1
    setCombo (osc1Wave,  "osc1Wave",  apvts, osc1WaveA, {"Sine","Triangle","Saw","Square","Noise","Wavetable"}, 2, this, laf.get());
    setCombo (osc1WT,    "osc1WavetableIndex", apvts, osc1WTA, {"Sine","Triangle","Saw","Square","Moog Saw","PWM Sweep","Brass","Soft Square","FM Bell","Vocal","Additive 1","Organ","Pluck","Chip","Noise WT"}, 0, this, laf.get());
    osc1Lev.setup ("osc1Level",        apvts, 0.0f, 1.0f, 0.01f, 0.7f,   "LEVEL",  "",     this, laf.get());
    osc1Pit.setup ("osc1Pitch",        apvts, -24.0f,24.0f,1.0f,0.0f,    "PITCH",  " st",  this, laf.get());
    osc1Fin.setup ("osc1FineTune",     apvts, -50.0f,50.0f,1.0f,0.0f,    "FINE",   " ct",  this, laf.get());
    osc1Pan.setup ("osc1Pan",          apvts, -1.0f, 1.0f, 0.01f,0.0f,   "PAN",    "",     this, laf.get());
    osc1Uni.setup ("osc1Unison",       apvts, 1.0f, 8.0f, 1.0f, 1.0f,    "UNISON", "",     this, laf.get());
    osc1Det.setup ("osc1Detune",       apvts, 0.0f, 50.0f,1.0f,0.0f,     "DETUNE", " ct",  this, laf.get());
    osc1PW.setup  ("osc1PulseWidth",    apvts, 0.01f,0.99f,0.01f,0.5f,   "PW",     "%",    this, laf.get());
    osc1Scn.setup ("osc1Scan",         apvts, 0.0f, 1.0f, 0.01f,0.0f,   "SCAN",   "",     this, laf.get());
    addAndMakeVisible (wf1);

    // Wire waveform preview to wave type changes (chain ComboBoxAttachment callback)
    {
        auto chainW1 = osc1Wave.onChange;
        osc1Wave.onChange = [chainW1, this]() { if (chainW1) chainW1(); wf1.setType(osc1Wave.getText()); };
        auto chainW1t = osc1WT.onChange;
        osc1WT.onChange = [chainW1t, this]() { if (chainW1t) chainW1t(); wf1.setType(osc1WT.getText()); };
    }

    // --- Osc 2
    setCombo (osc2Wave,  "osc2Wave",  apvts, osc2WaveA, {"Sine","Triangle","Saw","Square","Noise","Wavetable"}, 2, this, laf.get());
    setCombo (osc2WT,    "osc2WavetableIndex", apvts, osc2WTA, {"Sine","Triangle","Saw","Square","Moog Saw","PWM Sweep","Brass","Soft Square","FM Bell","Vocal","Additive 1","Organ","Pluck","Chip","Noise WT"}, 0, this, laf.get());
    osc2Lev.setup ("osc2Level",        apvts, 0.0f, 1.0f, 0.01f, 0.5f,   "LEVEL",  "",     this, laf.get());
    osc2Pit.setup ("osc2Pitch",        apvts, -24.0f,24.0f,1.0f,0.0f,    "PITCH",  " st",  this, laf.get());
    osc2Fin.setup ("osc2FineTune",     apvts, -50.0f,50.0f,1.0f,0.0f,    "FINE",   " ct",  this, laf.get());
    osc2Pan.setup ("osc2Pan",          apvts, -1.0f, 1.0f, 0.01f,0.0f,   "PAN",    "",     this, laf.get());
    osc2Uni.setup ("osc2Unison",       apvts, 1.0f, 8.0f, 1.0f, 1.0f,    "UNISON", "",     this, laf.get());
    osc2Det.setup ("osc2Detune",       apvts, 0.0f, 50.0f,1.0f,0.0f,     "DETUNE", " ct",  this, laf.get());
    osc2PW.setup  ("osc2PulseWidth",    apvts, 0.01f,0.99f,0.01f,0.5f,   "PW",     "%",    this, laf.get());
    osc2Scn.setup ("osc2Scan",         apvts, 0.0f, 1.0f, 0.01f,0.0f,   "SCAN",   "",     this, laf.get());
    addAndMakeVisible (wf2);
    // Sync toggle
    addAndMakeVisible(osc2SyncToggle);
    osc2SyncToggle.setClickingTogglesState(true);
    osc2SyncToggle.setLookAndFeel(laf.get());
    osc2SyncToggle.setButtonText("SYNC");
    osc2SyncToggle.setColour(juce::TextButton::buttonOnColourId, juce::Colour(ColourID::kSignalPath));
    osc2SyncToggleA = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, ParameterIDs::osc2Sync, osc2SyncToggle);

    {
        auto chainW2 = osc2Wave.onChange;
        osc2Wave.onChange = [chainW2, this]() { if (chainW2) chainW2(); wf2.setType(osc2Wave.getText()); };
        auto chainW2t = osc2WT.onChange;
        osc2WT.onChange = [chainW2t, this]() { if (chainW2t) chainW2t(); wf2.setType(osc2WT.getText()); };
    }

    // --- Osc 3
    setCombo (osc3Wave,  "osc3Wave",  apvts, osc3WaveA, {"Sine","Triangle","Saw","Square","Noise","Wavetable"}, 2, this, laf.get());
    setCombo (osc3WT,    "osc3WavetableIndex", apvts, osc3WTA, {"Sine","Triangle","Saw","Square","Moog Saw","PWM Sweep","Brass","Soft Square","FM Bell","Vocal","Additive 1","Organ","Pluck","Chip","Noise WT"}, 0, this, laf.get());
    osc3Lev.setup ("osc3Level",        apvts, 0.0f, 1.0f, 0.01f, 0.3f,   "LEVEL",  "",     this, laf.get());
    osc3Pit.setup ("osc3Pitch",        apvts, -24.0f,24.0f,1.0f,0.0f,    "PITCH",  " st",  this, laf.get());
    osc3Fin.setup ("osc3FineTune",     apvts, -50.0f,50.0f,1.0f,0.0f,    "FINE",   " ct",  this, laf.get());
    osc3Pan.setup ("osc3Pan",          apvts, -1.0f, 1.0f, 0.01f,0.0f,   "PAN",    "",     this, laf.get());
    osc3Uni.setup ("osc3Unison",       apvts, 1.0f, 8.0f, 1.0f, 1.0f,    "UNISON", "",     this, laf.get());
    osc3Det.setup ("osc3Detune",       apvts, 0.0f, 50.0f,1.0f,0.0f,     "DETUNE", " ct",  this, laf.get());
    osc3PW.setup  ("osc3PulseWidth",    apvts, 0.01f,0.99f,0.01f,0.5f,   "PW",     "%",    this, laf.get());
    osc3Scn.setup ("osc3Scan",         apvts, 0.0f, 1.0f, 0.01f,0.0f,   "SCAN",   "",     this, laf.get());
    addAndMakeVisible (wf3);
    // FM toggle
    addAndMakeVisible(osc3FMToggle);
    osc3FMToggle.setClickingTogglesState(true);
    osc3FMToggle.setLookAndFeel(laf.get());
    osc3FMToggle.setButtonText("FM");
    osc3FMToggle.setColour(juce::TextButton::buttonOnColourId, juce::Colour(ColourID::kSignalPath));
    osc3FMToggleA = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, ParameterIDs::osc3FM, osc3FMToggle);

    {
        auto chainW3 = osc3Wave.onChange;
        osc3Wave.onChange = [chainW3, this]() { if (chainW3) chainW3(); wf3.setType(osc3Wave.getText()); };
        auto chainW3t = osc3WT.onChange;
        osc3WT.onChange = [chainW3t, this]() { if (chainW3t) chainW3t(); wf3.setType(osc3WT.getText()); };
    }

    // Sub + Noise
    setCombo (subWave,   "subWave",  apvts, subWaveA,  {"Sine","Square"}, 0, this, laf.get());
    subLev.setup ("subLevel",         apvts, 0.0f, 1.0f, 0.01f, 0.0f,   "SUB",    "",     this, laf.get());
    subPit.setup ("subPitch",         apvts, -24.0f,0.0f,1.0f,-12.0f,   "S PIT",  " st",  this, laf.get());
    setCombo (noiseType, "noiseType", apvts, noiseTypeA, {"White","Pink"}, 0, this, laf.get());
    noiseLev.setup ("noiseLevel",     apvts, 0.0f, 1.0f, 0.01f, 0.0f,   "NOISE",  "",     this, laf.get());

    // Filter
    setCombo (filtType, "filterType", apvts, filtTypeA, {"LP 4-Pole","LP 2-Pole","HP 4-Pole","HP 2-Pole","Bandpass","Notch"}, 0, this, laf.get());
    filtCut.setup ("filterCutoff",     apvts, 20.0f,20000.0f,1.0f,1000.0f,"CUTOFF", " Hz",  this, laf.get());
    filtRes.setup ("filterResonance",  apvts, 0.0f, 1.0f, 0.01f, 0.0f,  "RESO",   "",     this, laf.get());
    filtDrv.setup ("filterDrive",      apvts, 0.0f, 1.0f, 0.01f, 0.0f,  "DRIVE",  "",     this, laf.get());
    filtKey.setup ("filterKeyTrack",   apvts, -1.0f, 1.0f, 0.01f, 0.0f, "KEY",    "",     this, laf.get());
    filtVel.setup ("filterVelTrack",   apvts, -1.0f, 1.0f, 0.01f, 0.0f, "VEL",    "",     this, laf.get());
    addAndMakeVisible (filterCurve);

    // Wire filter curve to knob/combo changes
    {
        auto updateFc = [this]() {
            filterCurve.setParams(
                filtType.getSelectedId() - 1,
                filtCut.slider.getValue(), filtRes.slider.getValue());
        };
        auto chainFt = filtType.onChange;
        filtType.onChange = [chainFt, updateFc]() { if (chainFt) chainFt(); updateFc(); };
        auto chainFc = filtCut.slider.onValueChange;
        filtCut.slider.onValueChange = [chainFc, updateFc]() { if (chainFc) chainFc(); updateFc(); };
        auto chainFr = filtRes.slider.onValueChange;
        filtRes.slider.onValueChange = [chainFr, updateFc]() { if (chainFr) chainFr(); updateFc(); };
    }

    // Amp Env
    ampA.setup  ("ampAttack",   apvts, 0.001f, 10.0f, 0.001f, 0.01f,  "ATT"," s", this, laf.get(), juce::Colour(ColourID::kModulation));
    ampD.setup  ("ampDecay",    apvts, 0.001f, 10.0f, 0.001f, 0.3f,   "DEC"," s", this, laf.get(), juce::Colour(ColourID::kModulation));
    ampS.setup  ("ampSustain",  apvts, 0.0f, 1.0f, 0.01f, 0.7f,       "SUS","",   this, laf.get(), juce::Colour(ColourID::kModulation));
    ampR.setup  ("ampRelease",  apvts, 0.001f, 10.0f, 0.001f, 0.3f,   "REL"," s", this, laf.get(), juce::Colour(ColourID::kModulation));
    ampVel.setup("ampVelSens",  apvts, 0.0f, 1.0f, 0.01f, 0.5f,       "VEL","",   this, laf.get(), juce::Colour(ColourID::kModulation));
    addAndMakeVisible (ampCurve);

    // Wire curve to knob changes (preserve APVTS attachment callback)
    auto wireCurveKnob = [this](KnobGroup& a, KnobGroup& d, KnobGroup& s, KnobGroup& r, EnvDisplay& curve) {
        auto updater = [&curve, &a, &d, &s, &r]() {
            curve.setParams(a.slider.getValue(), d.slider.getValue(),
                            s.slider.getValue(), r.slider.getValue());
        };
        auto chainA = a.slider.onValueChange;
        a.slider.onValueChange = [chainA, updater]() { if (chainA) chainA(); updater(); };
        auto chainD = d.slider.onValueChange;
        d.slider.onValueChange = [chainD, updater]() { if (chainD) chainD(); updater(); };
        auto chainS = s.slider.onValueChange;
        s.slider.onValueChange = [chainS, updater]() { if (chainS) chainS(); updater(); };
        auto chainR = r.slider.onValueChange;
        r.slider.onValueChange = [chainR, updater]() { if (chainR) chainR(); updater(); };
    };
    wireCurveKnob(ampA, ampD, ampS, ampR, ampCurve);

    // Filt Env
    fenvA.setup  ("filtAttack",   apvts, 0.001f, 10.0f, 0.001f, 0.01f,"ATT"," s", this, laf.get(), juce::Colour(ColourID::kModulation));
    fenvD.setup  ("filtDecay",    apvts, 0.001f, 10.0f, 0.001f, 0.3f, "DEC"," s", this, laf.get(), juce::Colour(ColourID::kModulation));
    fenvS.setup  ("filtSustain",  apvts, 0.0f, 1.0f, 0.01f, 0.0f,     "SUS","",   this, laf.get(), juce::Colour(ColourID::kModulation));
    fenvR.setup  ("filtRelease",  apvts, 0.001f, 10.0f, 0.001f, 0.3f, "REL"," s", this, laf.get(), juce::Colour(ColourID::kModulation));
    fenvAmt.setup("filtAmount",   apvts, -1.0f, 1.0f, 0.01f, 0.5f,    "AMT","",   this, laf.get(), juce::Colour(ColourID::kModulation));
    fenvVel.setup("filtVelSens",  apvts, 0.0f, 1.0f, 0.01f, 0.0f,     "VEL","",   this, laf.get(), juce::Colour(ColourID::kModulation));
    addAndMakeVisible (fenvCurve);

    wireCurveKnob(fenvA, fenvD, fenvS, fenvR, fenvCurve);

    // LFO 1
    setCombo (lfo1Wave, "lfo1Wave", apvts, lfo1WaveA, {"Sine","Triangle","Saw","Square","S&H"}, 0, this, laf.get());
    lfo1Rate.setup ("lfo1Rate",   apvts, 0.01f, 20.0f, 0.01f, 1.0f,  "RATE"," Hz",this, laf.get(), juce::Colour(ColourID::kModulation));
    lfo1Amt.setup  ("lfo1Amount", apvts, 0.0f, 1.0f, 0.01f, 0.0f,   "AMT", "",   this, laf.get(), juce::Colour(ColourID::kModulation));
    lfo1Del.setup  ("lfo1Delay",  apvts, 0.0f, 5.0f, 0.01f, 0.0f,   "DEL"," s",  this, laf.get(), juce::Colour(ColourID::kModulation));
    lfo1Fade.setup ("lfo1Fade",   apvts, 0.0f, 5.0f, 0.01f, 0.0f,   "FADE"," s", this, laf.get(), juce::Colour(ColourID::kModulation));

    // LFO 2
    setCombo (lfo2Wave, "lfo2Wave", apvts, lfo2WaveA, {"Sine","Triangle","Saw","Square","S&H"}, 1, this, laf.get());
    lfo2Rate.setup ("lfo2Rate",   apvts, 0.01f, 20.0f, 0.01f, 5.0f,  "RATE"," Hz",this, laf.get(), juce::Colour(ColourID::kModulation));
    lfo2Amt.setup  ("lfo2Amount", apvts, 0.0f, 1.0f, 0.01f, 0.0f,   "AMT", "",   this, laf.get(), juce::Colour(ColourID::kModulation));
    lfo2Del.setup  ("lfo2Delay",  apvts, 0.0f, 5.0f, 0.01f, 0.0f,   "DEL"," s",  this, laf.get(), juce::Colour(ColourID::kModulation));
    lfo2Fade.setup ("lfo2Fade",   apvts, 0.0f, 5.0f, 0.01f, 0.0f,   "FADE"," s", this, laf.get(), juce::Colour(ColourID::kModulation));

    // LFO 3
    setCombo (lfo3Wave, "lfo3Wave", apvts, lfo3WaveA, {"Sine","Triangle","Saw","Square","S&H"}, 0, this, laf.get());
    lfo3Rate.setup ("lfo3Rate",   apvts, 0.01f, 20.0f, 0.01f, 0.5f,  "RATE"," Hz",this, laf.get(), juce::Colour(ColourID::kModulation));
    lfo3Amt.setup  ("lfo3Amount", apvts, 0.0f, 1.0f, 0.01f, 0.0f,   "AMT", "",   this, laf.get(), juce::Colour(ColourID::kModulation));
    lfo3Del.setup  ("lfo3Delay",  apvts, 0.0f, 5.0f, 0.01f, 0.0f,   "DEL"," s",  this, laf.get(), juce::Colour(ColourID::kModulation));
    lfo3Fade.setup ("lfo3Fade",   apvts, 0.0f, 5.0f, 0.01f, 0.0f,   "FADE"," s", this, laf.get(), juce::Colour(ColourID::kModulation));

    // Arpeggiator
    addAndMakeVisible(arpToggle);
    arpToggle.setClickingTogglesState(true);
    arpToggle.setLookAndFeel(laf.get());
    arpToggle.setColour(juce::TextButton::buttonOnColourId, juce::Colour(ColourID::kArp));
    arpToggle.onClick = [this]() {
        bool on = arpToggle.getToggleState();
        arpToggle.setButtonText(on ? "OFF" : "ON");
        arpToggle.setColour(juce::TextButton::buttonColourId, on ? juce::Colour(ColourID::kArp) : juce::Colour(ColourID::kPanel));
    };
    arpToggleA = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, ParameterIDs::arpEnabled, arpToggle);
    setCombo(arpMode, "arpMode", apvts, arpModeA, {"Up", "Down", "Up-Down", "Random", "Order Played", "Custom"}, 0, this, laf.get());
    setCombo(arpRate, "arpRate", apvts, arpRateA, {"1/4", "1/8", "1/16", "1/8T", "1/16T", "1/8D"}, 1, this, laf.get());
    arpOctaves.setup("arpOctaves", apvts, 1.0f, 4.0f, 1.0f, 1.0f, "OCTAVES", "", this, laf.get());
    arpGate.setup("arpGate", apvts, 0.1f, 1.0f, 0.01f, 0.8f, "GATE", "", this, laf.get());
    arpSteps.setup("arpSteps", apvts, 2.0f, 16.0f, 1.0f, 8.0f, "STEPS", "", this, laf.get());
    arpSwing.setup("arpSwing", apvts, 0.0f, 1.0f, 0.01f, 0.0f, "SWING", "", this, laf.get());

    // Step sequencer grid (16 steps)
    addAndMakeVisible(stepPanel);
    stepPanel.setVisible(false);  // hidden until Custom mode selected
    for (int i = 0; i < 16; ++i)
    {
        auto& slider = arpStepSliders[i];
        auto& toggle = arpStepToggles[i];
        auto& label  = arpStepLabels[i];

        addChildComponent(slider);
        addChildComponent(toggle);
        addChildComponent(label);

        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setRange(-24.0, 24.0, 1.0);
        slider.setValue(0.0);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setLookAndFeel(laf.get());
        slider.setColour(juce::Slider::trackColourId, juce::Colour(ColourID::kArp));
        slider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xFF1A1A2E));

        toggle.setLookAndFeel(laf.get());
        toggle.setColour(juce::ToggleButton::tickColourId, juce::Colour(ColourID::kArp));
        toggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xFF333344));

        label.setText(juce::String(i + 1), juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::FontOptions(9.0f));
        label.setColour(juce::Label::textColourId, juce::Colour(0xFF666677));

        arpStepSliderA[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, ParameterIDs::arpStepOffset(i), slider);
        arpStepToggleA[i] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, ParameterIDs::arpStepEnable(i), toggle);
    }

    // Show/hide step grid based on arp mode
    arpMode.onChange = [this]() {
        bool isCustom = (arpMode.getSelectedId() - 1) == 5;  // "Custom" is index 5
        stepPanel.setVisible(isCustom);
        for (int i = 0; i < 16; ++i)
        {
            arpStepSliders[i].setVisible(isCustom);
            arpStepToggles[i].setVisible(isCustom);
            arpStepLabels[i].setVisible(isCustom);
        }
        resized();  // relayout
    };

    // FX + Macro placeholder knobs (visual only)
    auto setupPh = [&](KnobGroup& k, const juce::String& txt, const juce::String& suf, float def, juce::Colour col) {
        addAndMakeVisible(k.slider); addAndMakeVisible(k.label);
        k.slider.setLookAndFeel(laf.get());
        k.slider.setColour(juce::Slider::rotarySliderFillColourId, col);
        k.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        k.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 20);
        k.slider.setTextValueSuffix(suf);
        k.slider.setRange(0.0f, 1.0f, 0.01f); k.slider.setValue(def);
        k.label.setText(txt, juce::dontSendNotification);
        k.label.setJustificationType(juce::Justification::centred);
        k.label.setFont(juce::FontOptions(9.0f));
        k.label.setColour(juce::Label::textColourId, juce::Colour(ColourID::kTextMuted));
    };
    // Real FX controls — each with toggle + knobs wired to APVTS
    auto setupFXToggle = [&](juce::TextButton& btn, const char* paramID) {
        addAndMakeVisible(btn);
        btn.setClickingTogglesState(true);
        btn.setLookAndFeel(laf.get());
        btn.setColour(juce::TextButton::buttonColourId, juce::Colour(ColourID::kEffects));
        btn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(ColourID::kEffects));
        btn.setButtonText("OFF");
        btn.onClick = [&btn]() {
            btn.setButtonText(btn.getToggleState() ? "ON" : "OFF");
        };
    };
    setupFXToggle(chToggle, "chorusOn");
    chToggleA = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, ParameterIDs::chorusOn, chToggle);
    chRate.setup("chorusRate", apvts, 0.1f, 5.0f, 0.01f, 1.0f, "RATE", "Hz", this, laf.get(), juce::Colour(ColourID::kEffects));
    chDepth.setup("chorusDepth", apvts, 0.0f, 1.0f, 0.01f, 0.5f, "DEPTH", "", this, laf.get(), juce::Colour(ColourID::kEffects));
    chMix.setup("chorusMix", apvts, 0.0f, 1.0f, 0.01f, 0.3f, "MIX", "", this, laf.get(), juce::Colour(ColourID::kEffects));

    setupFXToggle(flToggle, "flangerOn");
    flToggleA = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, ParameterIDs::flangerOn, flToggle);
    flRate.setup("flangerRate", apvts, 0.05f, 5.0f, 0.01f, 0.5f, "RATE", "Hz", this, laf.get(), juce::Colour(ColourID::kEffects));
    flDepth.setup("flangerDepth", apvts, 0.0f, 1.0f, 0.01f, 0.5f, "DEPTH", "", this, laf.get(), juce::Colour(ColourID::kEffects));
    flFb.setup("flangerFeedback", apvts, 0.0f, 1.0f, 0.01f, 0.5f, "FB", "", this, laf.get(), juce::Colour(ColourID::kEffects));
    flMix.setup("flangerMix", apvts, 0.0f, 1.0f, 0.01f, 0.5f, "MIX", "", this, laf.get(), juce::Colour(ColourID::kEffects));

    setupFXToggle(phToggle, "phaserOn");
    phToggleA = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, ParameterIDs::phaserOn, phToggle);
    phRate.setup("phaserRate", apvts, 0.05f, 5.0f, 0.01f, 0.3f, "RATE", "Hz", this, laf.get(), juce::Colour(ColourID::kEffects));
    phDepth.setup("phaserDepth", apvts, 0.0f, 1.0f, 0.01f, 0.5f, "DEPTH", "", this, laf.get(), juce::Colour(ColourID::kEffects));
    phFb.setup("phaserFeedback", apvts, 0.0f, 1.0f, 0.01f, 0.5f, "FB", "", this, laf.get(), juce::Colour(ColourID::kEffects));
    phMix.setup("phaserMix", apvts, 0.0f, 1.0f, 0.01f, 0.5f, "MIX", "", this, laf.get(), juce::Colour(ColourID::kEffects));

    setupFXToggle(dlyToggle, "delayOn");
    dlyToggleA = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, ParameterIDs::delayOn, dlyToggle);
    dlyTimeL.setup("delayTimeL", apvts, 0.02f, 2.0f, 0.01f, 0.5f, "TIME L", "s", this, laf.get(), juce::Colour(ColourID::kEffects));
    dlyTimeR.setup("delayTimeR", apvts, 0.02f, 2.0f, 0.01f, 0.375f, "TIME R", "s", this, laf.get(), juce::Colour(ColourID::kEffects));
    dlyFb2.setup("delayFeedback", apvts, 0.0f, 1.0f, 0.01f, 0.4f, "FB", "", this, laf.get(), juce::Colour(ColourID::kEffects));
    dlyWet2.setup("delayWet", apvts, 0.0f, 1.0f, 0.01f, 0.3f, "WET", "", this, laf.get(), juce::Colour(ColourID::kEffects));

    setupFXToggle(revToggle, "reverbOn");
    revToggleA = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, ParameterIDs::reverbOn, revToggle);
    revSize2.setup("reverbSize", apvts, 0.1f, 1.0f, 0.01f, 0.5f, "SIZE", "", this, laf.get(), juce::Colour(ColourID::kEffects));
    revDamp.setup("reverbDamp", apvts, 0.0f, 1.0f, 0.01f, 0.5f, "DAMP", "", this, laf.get(), juce::Colour(ColourID::kEffects));
    revWet2.setup("reverbWet", apvts, 0.0f, 1.0f, 0.01f, 0.2f, "WET", "", this, laf.get(), juce::Colour(ColourID::kEffects));

    // FX sub-labels
    auto setupFXLabel = [&](juce::Label& lbl, const juce::String& txt) {
        addAndMakeVisible(lbl);
        lbl.setText(txt, juce::dontSendNotification);
        lbl.setFont(juce::FontOptions(8.0f, juce::Font::bold));
        lbl.setColour(juce::Label::textColourId, juce::Colour(ColourID::kEffects));
        lbl.setJustificationType(juce::Justification::centred);
    };
    setupFXLabel(chLabel, "CHORUS");  setupFXLabel(flLabel, "FLANGER");
    setupFXLabel(phLabel, "PHASER");  setupFXLabel(dlyLabel2, "DELAY");
    setupFXLabel(revLabel2, "REVERB");

    // Macro placeholder knobs (for future use)
    setupPh(macro1, "M1", "", 0.0f, juce::Colour(ColourID::kModulation));  setupPh(macro2, "M2", "", 0.0f, juce::Colour(ColourID::kModulation));
    setupPh(macro3, "M3", "", 0.0f, juce::Colour(ColourID::kModulation));  setupPh(macro4, "M4", "", 0.0f, juce::Colour(ColourID::kModulation));

    // Mod matrix label
    addAndMakeVisible (modLabel);
    modLabel.setText ("8-slot Mod Matrix  —  right-click knobs to assign", juce::dontSendNotification);
    modLabel.setJustificationType (juce::Justification::centred);
    modLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF555566));
    modLabel.setFont (juce::FontOptions (10.0f));

    // === Wrap all content in a scrollable Viewport ===
    // Move all children from 'this' to contentComp
    juce::Array<juce::Component*> kids = getChildren();
    for (auto* child : kids)
        contentComp.addAndMakeVisible(*child);
    
    contentComp.setSize(1050, 1500);
    viewport.setViewedComponent(&contentComp, false);
    viewport.setScrollBarsShown(true, false);  // vertical scroll only
    addAndMakeVisible(viewport);
    setSize(1050, 780);  // editor window fits on 1080p screen
}

//==============================================================================
void AnalogSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(ColourID::kBg));

    // top bar
    auto hdr = getLocalBounds().removeFromTop (48);
    g.setColour (juce::Colour(ColourID::kSurface));
    g.fillRect (hdr);
    g.setColour (juce::Colour(ColourID::kBorder).withAlpha(0.6f));
    g.drawLine (0, hdr.getBottom(), getWidth(), hdr.getBottom(), 1.0f);

    g.setColour (juce::Colour(ColourID::kSignalPath).withAlpha (0.9f));
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("ANALOG SYNTH", hdr.removeFromLeft (180).reduced (14, 0),
                juce::Justification::centredLeft);
}

//==============================================================================
static juce::Rectangle<int> knobRect (juce::Rectangle<int> area, int ks, int lh)
{
    auto k = area.removeFromTop (ks).reduced (2);
    auto l = area.removeFromTop (lh).reduced (2);
    return k.withBottom (l.getBottom());
}

void AnalogSynthAudioProcessorEditor::resized()
{
    if (!built) { buildUI(); built = true; }

    viewport.setBounds(getLocalBounds().withTrimmedTop(48));
    contentComp.setSize(1050, 1700);  // taller for 48px knobs + LFO3 + signal flow

    auto b    = contentComp.getLocalBounds().reduced (10);
    auto hdr  = b.removeFromTop (48);

    // Signal flow diagram
    signalFlow.setBounds(b.removeFromTop(42).reduced(0, 2));
    b.removeFromTop(6);

    // global knobs (top-right)

    // Preset selector (in header, next to global knobs)
    auto pset = hdr.removeFromRight(260).reduced(4);
    presetBox.setBounds(pset.removeFromLeft(120).reduced(2));
    auto gb2 = pset;
    auto gb = gb2;
    int  gw = gb.getWidth() / 3;
    masterGain.slider.setBounds (gb.removeFromLeft (gw).reduced (2));
    polyphony.slider.setBounds   (gb.removeFromLeft (gw).reduced (2));
    pitchBend.slider.setBounds   (gb.reduced (2));

    b.removeFromTop (4);

    // === LEFT ===
    auto L = b.removeFromLeft (b.getWidth() * 3 / 5);
    auto R = b;

    // ----- OSCILLATORS -----
    auto oscB = L.removeFromTop (380);
    oscPanel.setBounds (oscB);
    auto oi = oscB.reduced (8, 30);
    int  ocw = (oi.getWidth() - 2 * 4) / 3;

    auto layOsc = [&](juce::ComboBox& wv, juce::ComboBox& wt, WaveformPreview& wf,
                      KnobGroup& lv, KnobGroup& pt, KnobGroup& fn, KnobGroup& pn,
                      KnobGroup& un, KnobGroup& dt, KnobGroup& pw, KnobGroup& sc,
                      juce::Rectangle<int> c)
    {
        wv.setBounds (c.removeFromTop (kComboH).reduced (1));
        wt.setBounds (c.removeFromTop (kComboH).reduced (1));
        wf.setBounds (c.removeFromTop (28).reduced (2));

        // Knobs: 2 rows of 4
        auto r1 = c.removeFromTop (56);
        lv.slider.setBounds (r1.removeFromLeft (ocw).reduced (1));
        pt.slider.setBounds (r1.removeFromLeft (ocw).reduced (1));
        fn.slider.setBounds (r1.removeFromLeft (ocw).reduced (1));
        pn.slider.setBounds (r1.reduced (1));

        auto r2 = c.removeFromTop (56);
        un.slider.setBounds (r2.removeFromLeft (ocw).reduced (1));
        dt.slider.setBounds (r2.removeFromLeft (ocw).reduced (1));
        pw.slider.setBounds (r2.removeFromLeft (ocw).reduced (1));
        sc.slider.setBounds (r2.reduced (1));
    };

    auto o1 = oi.removeFromLeft (ocw); oi.removeFromLeft (4);
    auto o2 = oi.removeFromLeft (ocw); oi.removeFromLeft (4);
    auto o3 = oi;
    layOsc (osc1Wave, osc1WT, wf1, osc1Lev,osc1Pit,osc1Fin,osc1Pan,osc1Uni,osc1Det,osc1PW,osc1Scn, o1);
    layOsc (osc2Wave, osc2WT, wf2, osc2Lev,osc2Pit,osc2Fin,osc2Pan,osc2Uni,osc2Det,osc2PW,osc2Scn, o2);
    layOsc (osc3Wave, osc3WT, wf3, osc3Lev,osc3Pit,osc3Fin,osc3Pan,osc3Uni,osc3Det,osc3PW,osc3Scn, o3);

    // Sync/FM toggles — place at bottom of each osc column
    osc2SyncToggle.setBounds(o2.removeFromBottom(18).reduced(2).removeFromRight(45));
    osc3FMToggle.setBounds(o3.removeFromBottom(18).reduced(2).removeFromRight(45));

    // Sub + Noise (below osc)
    auto sn = oscB.reduced (6, 0).removeFromBottom (48);
    auto subRow  = sn.removeFromTop (22);
    subWave.setBounds  (subRow.removeFromLeft (80).reduced (2));
    subLev.slider.setBounds  (subRow.removeFromLeft (40).reduced (2));
    subPit.slider.setBounds  (subRow.removeFromLeft (40).reduced (2));
    sn.removeFromTop (2);
    noiseType.setBounds (sn.removeFromLeft (70).reduced (2));
    noiseLev.slider.setBounds (sn.removeFromLeft (40).reduced (2));

    L.removeFromTop (4);

    // ----- FILTER -----
    auto filtB = L.removeFromTop (240);
    filtPanel.setBounds (filtB);
    auto fi = filtB.reduced (8, 30);

    // Filter response curve
    auto fcv = fi.removeFromTop (74).reduced (4);
    filterCurve.setBounds (fcv);
    filterCurve.setParams (
        filtType.getSelectedId() - 1,
        apvts.getRawParameterValue("filterCutoff")->load(),
        apvts.getRawParameterValue("filterResonance")->load()
    );

    filtType.setBounds (fi.removeFromTop (kComboH).reduced (2));
    auto fkr = fi;
    int  fkw = fi.getWidth() / 5;
    auto placeK = [&](KnobGroup& k) {
        k.slider.setBounds (fkr.removeFromLeft (fkw).reduced (2));
        k.label.setBounds (k.slider.getBounds().withY (k.slider.getBottom()).withHeight (kLabelH));
    };
    placeK (filtCut); placeK (filtRes); placeK (filtDrv); placeK (filtKey); placeK (filtVel);

    // === RIGHT ===
    // ----- AMP ENV -----
    auto ampB = R.removeFromTop (220);
    ampPanel.setBounds (ampB);
    auto ai = ampB.reduced (8, 30);

    // ADSR curve display
    auto curveA = ai.removeFromTop (62).reduced (4);
    ampCurve.setBounds (curveA);
    ampCurve.setParams (
        apvts.getRawParameterValue("ampAttack")->load(),
        apvts.getRawParameterValue("ampDecay")->load(),
        apvts.getRawParameterValue("ampSustain")->load(),
        apvts.getRawParameterValue("ampRelease")->load()
    );

    int  akw = ai.getWidth() / 5;
    auto arow = ai.removeFromTop (56);
    ampA.slider.setBounds (arow.removeFromLeft (akw).reduced (2));
    ampD.slider.setBounds (arow.removeFromLeft (akw).reduced (2));
    ampS.slider.setBounds (arow.removeFromLeft (akw).reduced (2));
    ampR.slider.setBounds (arow.removeFromLeft (akw).reduced (2));
    ampVel.slider.setBounds (arow.reduced (2));
    // labels
    int ly = ampA.slider.getBottom();
    auto lrow = ai.removeFromTop (kLabelH);
    ampA.label.setBounds (lrow.removeFromLeft (akw).reduced (2).withY (ly));
    ampD.label.setBounds (lrow.removeFromLeft (akw).reduced (2).withY (ly));
    ampS.label.setBounds (lrow.removeFromLeft (akw).reduced (2).withY (ly));
    ampR.label.setBounds (lrow.removeFromLeft (akw).reduced (2).withY (ly));
    ampVel.label.setBounds (lrow.reduced (2).withY (ly));

    R.removeFromTop (4);

    // ----- FILT ENV -----
    auto fenvB = R.removeFromTop (220);
    fenvPanel.setBounds (fenvB);
    auto fei = fenvB.reduced (8, 30);

    // ADSR curve display
    auto fcurveA = fei.removeFromTop (62).reduced (4);
    fenvCurve.setBounds (fcurveA);
    fenvCurve.setParams (
        apvts.getRawParameterValue("filtAttack")->load(),
        apvts.getRawParameterValue("filtDecay")->load(),
        apvts.getRawParameterValue("filtSustain")->load(),
        apvts.getRawParameterValue("filtRelease")->load()
    );

    int  few = fei.getWidth() / 5;
    auto ferow = fei.removeFromTop (56);
    fenvA.slider.setBounds (ferow.removeFromLeft (few).reduced (2));
    fenvD.slider.setBounds (ferow.removeFromLeft (few).reduced (2));
    fenvS.slider.setBounds (ferow.removeFromLeft (few).reduced (2));
    fenvR.slider.setBounds (ferow.removeFromLeft (few).reduced (2));
    fenvAmt.slider.setBounds (ferow.removeFromLeft (few).reduced (2));
    // labels + row 2
    int fly = fenvA.slider.getBottom();
    auto flrow = fei.removeFromTop (kLabelH);
    fenvA.label.setBounds (flrow.removeFromLeft (few).reduced (2).withY (fly));
    fenvD.label.setBounds (flrow.removeFromLeft (few).reduced (2).withY (fly));
    fenvS.label.setBounds (flrow.removeFromLeft (few).reduced (2).withY (fly));
    fenvR.label.setBounds (flrow.removeFromLeft (few).reduced (2).withY (fly));
    fenvAmt.label.setBounds (flrow.removeFromLeft (few).reduced (2).withY (fly));

    R.removeFromTop (4);

    // ----- LFOs (3 columns) -----
    auto lfoB = R.removeFromTop (140);
    lfoPanel.setBounds (lfoB);
    auto li = lfoB.reduced (8, 30);
    auto lThirdW = (li.getWidth() - 2 * 4) / 3;

    auto layLFO = [&](juce::ComboBox& wv, KnobGroup& rt, KnobGroup& am, KnobGroup& dl, KnobGroup& fd,
                      juce::Rectangle<int> c)
    {
        wv.setBounds (c.removeFromTop (kComboH).reduced (2));
        auto kr = c.removeFromTop (56);
        int  kw = kr.getWidth() / 4;
        rt.slider.setBounds (kr.removeFromLeft (kw).reduced (1));
        am.slider.setBounds (kr.removeFromLeft (kw).reduced (1));
        dl.slider.setBounds (kr.removeFromLeft (kw).reduced (1));
        fd.slider.setBounds (kr.reduced (1));
    };

    layLFO (lfo1Wave, lfo1Rate, lfo1Amt, lfo1Del, lfo1Fade, li.removeFromLeft (lThirdW));
    li.removeFromLeft (4);
    layLFO (lfo2Wave, lfo2Rate, lfo2Amt, lfo2Del, lfo2Fade, li.removeFromLeft (lThirdW));
    li.removeFromLeft (4);
    layLFO (lfo3Wave, lfo3Rate, lfo3Amt, lfo3Del, lfo3Fade, li);


    // ----- ARPEGGIATOR -----
    R.removeFromTop(4);
    auto arpB = R.removeFromTop(130);
    arpPanel.setBounds(arpB);
    auto ari = arpB.reduced(8, 30);
    arpToggle.setBounds(ari.removeFromTop(22).removeFromLeft(50).reduced(2));
    auto arcr = ari.removeFromTop(kComboH);
    arpMode.setBounds(arcr.removeFromLeft(arcr.getWidth() / 2 - 2).reduced(1));
    arpRate.setBounds(arcr.removeFromLeft(arcr.getWidth() / 2).reduced(1));
    auto arkr = ari.removeFromTop(56);
    int akw4 = arkr.getWidth() / 4;
    arpOctaves.slider.setBounds(arkr.removeFromLeft(akw4).reduced(1));
    arpGate.slider.setBounds(arkr.removeFromLeft(akw4).reduced(1));
    arpSteps.slider.setBounds(arkr.removeFromLeft(akw4).reduced(1));
    arpSwing.slider.setBounds(arkr.reduced(1));

    // ----- STEP SEQUENCER (Custom mode) -----
    if (stepPanel.isVisible())
    {
        R.removeFromTop(2);
        auto stepB = R.removeFromTop(120);
        stepPanel.setBounds(stepB);
        auto si = stepB.reduced(6, 26);
        int colW = si.getWidth() / 16;
        for (int i = 0; i < 16; ++i)
        {
            auto col = si.removeFromLeft(colW).reduced(1);
            arpStepLabels[i].setBounds(col.removeFromTop(14));
            arpStepToggles[i].setBounds(col.removeFromBottom(16).reduced(1, 0));
            arpStepSliders[i].setBounds(col.reduced(1, 2));
        }
    }
    // ----- FX -----
    R.removeFromTop(4);
    auto fxB = R.removeFromTop(260);  // expanded for 5 FX rows
    fxPanel.setBounds(fxB);
    auto fxi = fxB.reduced(6, 26);
    
    auto layFXRow = [&](juce::Label& lbl, juce::TextButton& toggle, 
                        KnobGroup** knobs, int numKnobs, juce::Rectangle<int> row)
    {
        lbl.setBounds(row.removeFromLeft(55).reduced(2, 4));
        toggle.setBounds(row.removeFromRight(32).reduced(2));
        int kw = row.getWidth() / numKnobs;
        for (int i = 0; i < numKnobs; ++i)
            knobs[i]->slider.setBounds(row.removeFromLeft(kw).reduced(1));
    };
    
    KnobGroup* chKnobs[] = {&chRate, &chDepth, &chMix};
    KnobGroup* flKnobs[] = {&flRate, &flDepth, &flFb, &flMix};
    KnobGroup* phKnobs[] = {&phRate, &phDepth, &phFb, &phMix};
    KnobGroup* dlyKnobs[] = {&dlyTimeL, &dlyTimeR, &dlyFb2, &dlyWet2};
    KnobGroup* revKnobs[] = {&revSize2, &revDamp, &revWet2};
    
    layFXRow(chLabel, chToggle, chKnobs, 3, fxi.removeFromTop(44).reduced(0, 2));
    layFXRow(flLabel, flToggle, flKnobs, 4, fxi.removeFromTop(44).reduced(0, 2));
    layFXRow(phLabel, phToggle, phKnobs, 4, fxi.removeFromTop(44).reduced(0, 2));
    layFXRow(dlyLabel2, dlyToggle, dlyKnobs, 4, fxi.removeFromTop(44).reduced(0, 2));
    layFXRow(revLabel2, revToggle, revKnobs, 3, fxi.removeFromTop(44).reduced(0, 2));

    // ----- MACRO -----
    R.removeFromTop(4);
    auto macB = R.removeFromTop(70);
    macroPanel.setBounds(macB);
    auto mac = macB.reduced(8, 30);
    int  mkw = mac.getWidth() / 4;
    macro1.slider.setBounds(mac.removeFromLeft(mkw).reduced(2));
    macro2.slider.setBounds(mac.removeFromLeft(mkw).reduced(2));
    macro3.slider.setBounds(mac.removeFromLeft(mkw).reduced(2));
    macro4.slider.setBounds(mac.reduced(2));

    // ----- MOD MATRIX -----
    R.removeFromTop (4);
    modPanel.setBounds (R);
    modLabel.setBounds (R.reduced (8, 26).removeFromBottom (20));
}

//==============================================================================
// Presets
//==============================================================================
void AnalogSynthAudioProcessorEditor::initPresets()
{
    juce::StringArray names = {
        "Init Saw",
        "Warm Pad",
        "Pluck Bass",
        "Bright Lead",
        "Dark Ambient",
        "Arp Ready",
        "Custom Pattern"
    };
    for (int i = 0; i < names.size(); ++i)
        presetBox.addItem(names[i], i + 1);
    presetBox.setSelectedId(1, juce::dontSendNotification);
    presetBox.onChange = [this]() {
        loadPreset(presetBox.getSelectedId() - 1);
    };
}

void AnalogSynthAudioProcessorEditor::loadPreset(int idx)
{
    auto setF = [&](const char* id, float v) {
        if (auto* p = apvts.getParameter(id))
            p->setValueNotifyingHost(v);
    };
    auto setI = [&](const char* id, int v) {
        if (auto* p = apvts.getParameter(id))
            p->setValueNotifyingHost(p->convertTo0to1((float)v));
    };

    // Reset to defaults first
    // Osc waves to Saw
    setI("osc1Wave", 2); setI("osc2Wave", 2); setI("osc3Wave", 2);
    setF("osc1Level", 0.7f); setF("osc2Level", 0.5f); setF("osc3Level", 0.3f);
    setF("osc1Pitch", 0); setF("osc2Pitch", 0); setF("osc3Pitch", 0);
    setF("osc1FineTune", 0); setF("osc2FineTune", 0); setF("osc3FineTune", 0);
    setF("osc1Pan", 0); setF("osc2Pan", 0); setF("osc3Pan", 0);
    setI("osc1Unison", 1); setI("osc2Unison", 1); setI("osc3Unison", 1);
    setF("osc1Detune", 0); setF("osc2Detune", 0); setF("osc3Detune", 0);
    setF("osc1PulseWidth", 0.5f); setF("osc2PulseWidth", 0.5f); setF("osc3PulseWidth", 0.5f);
    setI("osc1WavetableIndex", 0); setI("osc2WavetableIndex", 0); setI("osc3WavetableIndex", 0);
    setF("osc1Scan", 0); setF("osc2Scan", 0); setF("osc3Scan", 0);
    setF("subLevel", 0); setF("noiseLevel", 0);
    setI("filterType", 0);  // LP 4-Pole
    setF("filterCutoff", 1000.0f); setF("filterResonance", 0);
    setF("filterDrive", 0); setF("filterKeyTrack", 0); setF("filterVelTrack", 0);
    setF("ampAttack", 0.01f); setF("ampDecay", 0.3f); setF("ampSustain", 0.7f); setF("ampRelease", 0.3f);
    setF("ampVelSens", 0.5f);
    setF("filtAttack", 0.01f); setF("filtDecay", 0.3f); setF("filtSustain", 0); setF("filtRelease", 0.3f);
    setF("filtAmount", 0.5f); setF("filtVelSens", 0);
    setI("lfo1Wave", 0); setF("lfo1Rate", 1.0f); setF("lfo1Amount", 0); setF("lfo1Delay", 0); setF("lfo1Fade", 0);
    setI("lfo2Wave", 0); setF("lfo2Rate", 5.0f); setF("lfo2Amount", 0); setF("lfo2Delay", 0); setF("lfo2Fade", 0);
    setF("masterGain", 0.5f);
    setI("polyphony", 8);
    setF("pitchBendRange", 2.0f);

    // Arp defaults
    setI("arpEnabled", 0);
    setI("arpMode", 0); setF("arpRate", 1); setI("arpOctaves", 1); setF("arpGate", 0.8f);
    setI("arpSteps", 8); setF("arpSwing", 0.0f);
    for (int i = 0; i < 16; ++i)
    {
        setI(("arpS" + juce::String(i+1) + "Off").toRawUTF8(), 0);
        setI(("arpS" + juce::String(i+1) + "En").toRawUTF8(), (i < 8) ? 1 : 0);
    }

    switch (idx)
    {
    case 0: // Init Saw — default (already set)
        break;
    case 1: // Warm Pad
        setI("osc1Wave", 0); setI("osc2Wave", 1); setI("osc3Wave", 4);  // Sine, Triangle, Noise
        setF("osc1Level", 0.5f); setF("osc2Level", 0.4f); setF("osc3Level", 0.15f);
        setF("osc2Pitch", -12); setF("osc3Pitch", 7);
        setF("osc2Detune", 7); setF("osc3Detune", 12);
        setF("filterCutoff", 800.0f); setF("filterResonance", 0.3f);
        setF("ampAttack", 0.3f); setF("ampDecay", 0.8f); setF("ampSustain", 0.6f); setF("ampRelease", 1.5f);
        setI("lfo1Wave", 0); setF("lfo1Rate", 0.3f); setF("lfo1Amount", 0.15f);
        setF("filtAttack", 0.5f); setF("filtDecay", 1.0f); setF("filtAmount", 0.3f);
        break;
    case 2: // Pluck Bass
        setI("osc1Wave", 2); // Saw
        setF("osc1Level", 0.8f);
        setF("osc1Pitch", -12);
        setF("filterCutoff", 400.0f); setF("filterResonance", 0.4f); setF("filterDrive", 0.3f);
        setF("ampAttack", 0.001f); setF("ampDecay", 0.15f); setF("ampSustain", 0.1f); setF("ampRelease", 0.1f);
        setF("filtAttack", 0.001f); setF("filtDecay", 0.1f); setF("filtAmount", 0.6f);
        setF("ampVelSens", 0.8f);
        break;
    case 3: // Bright Lead
        setI("osc1Wave", 2); setI("osc2Wave", 2); // Both Saw
        setF("osc1Level", 0.7f); setF("osc2Level", 0.5f);
        setF("osc2Pitch", 7); setF("osc2Detune", 5);
        setI("osc1Unison", 3); setF("osc1Detune", 8);
        setF("filterCutoff", 3000.0f); setF("filterResonance", 0.2f);
        setF("ampAttack", 0.005f); setF("ampDecay", 0.2f); setF("ampSustain", 0.6f); setF("ampRelease", 0.3f);
        setF("masterGain", 0.6f);
        setF("pitchBendRange", 12.0f);
        break;
    case 4: // Dark Ambient
        setI("osc1Wave", 4); setI("osc2Wave", 4); // Noise for texture
        setI("osc3Wave", 5); // Wavetable
        setF("osc1Level", 0.1f); setF("osc2Level", 0.1f); setF("osc3Level", 0.6f);
        setI("osc3WavetableIndex", 9); // Vocal
        setF("osc3Pitch", -12);
        setF("filterCutoff", 300.0f); setF("filterResonance", 0.6f);
        setF("ampAttack", 1.0f); setF("ampDecay", 2.0f); setF("ampSustain", 0.5f); setF("ampRelease", 3.0f);
        setI("lfo1Wave", 0); setF("lfo1Rate", 0.2f); setF("lfo1Amount", 0.3f);
        setF("filtAmount", 0.5f);
        setF("noiseLevel", 0.05f);
        break;
    case 5: // Arp Ready
        setI("osc1Wave", 2); setI("osc2Wave", 1);
        setF("osc1Level", 0.6f); setF("osc2Level", 0.4f);
        setI("osc1Unison", 2); setF("osc1Detune", 5);
        setF("filterCutoff", 1200.0f); setF("filterResonance", 0.15f);
        setF("ampAttack", 0.01f); setF("ampDecay", 0.2f); setF("ampSustain", 0.5f); setF("ampRelease", 0.2f);
        setI("arpEnabled", 1);
        setI("arpMode", 2); // Up-Down
        setI("arpOctaves", 2);
        setF("arpGate", 0.6f);
        setF("arpSwing", 0.3f);  // added swing
        setF("filtAmount", 0.2f);
        break;
    case 6: // Custom Pattern (step sequencer)
        setI("osc1Wave", 2); setF("osc1Level", 0.7f);
        setF("filterCutoff", 800.0f); setF("filterResonance", 0.1f);
        setF("ampAttack", 0.01f); setF("ampDecay", 0.15f); setF("ampSustain", 0.4f); setF("ampRelease", 0.1f);
        setI("arpEnabled", 1);
        setI("arpMode", 5); // Custom
        setI("arpSteps", 8);
        setF("arpSwing", 0.0f);
        setF("arpGate", 0.5f);
        // Set step pattern: descending arp with varied offsets
        // Steps: 0, +7, +5, +3, +7, 0, +5, +12
        setI("arpS1Off", 0);  setI("arpS2Off", 7);  setI("arpS3Off", 5);  setI("arpS4Off", 3);
        setI("arpS5Off", 7);  setI("arpS6Off", 0);  setI("arpS7Off", 5);  setI("arpS8Off", 12);
        // Enable first 8 steps, disable 9-16
        for (int i = 0; i < 8; ++i)  setI(("arpS" + juce::String(i+1) + "En").toRawUTF8(), 1);
        for (int i = 8; i < 16; ++i) setI(("arpS" + juce::String(i+1) + "En").toRawUTF8(), 0);
        break;
    }
}
