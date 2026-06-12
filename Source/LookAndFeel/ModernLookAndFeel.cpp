#include "ModernLookAndFeel.h"

ModernLookAndFeel::ModernLookAndFeel()
{
    modernFont = juce::Typeface::createSystemTypefaceFor(
        juce::BinaryData::getNamedResource("modern_font", nullptr));
    if (!modernFont) {
        modernFont = juce::Font("Inter", 12.0f, juce::Font::plain).getTypeface();
    }
    
    setColourScheme(ColourScheme::Dark);
}

void ModernLookAndFeel::setColourScheme(ColourScheme scheme)
{
    currentScheme = scheme;
    colours = getSchemeColours(scheme);
}

ModernLookAndFeel::Colours ModernLookAndFeel::getSchemeColours(ColourScheme scheme)
{
    Colours c;
    switch (scheme) {
        case ColourScheme::Dark:
            c.bg = juce::Colour(0xFF1E1E2E);
            c.bgAlt = juce::Colour(0xFF25253A);
            c.accent = juce::Colour(0xFF00D4AA); // Teal
            c.accentAlt = juce::Colour(0xFF6C5CE7); // Purple
            c.text = juce::Colour(0xFFE0E0E0);
            c.textDim = juce::Colour(0xFF8A8A9A);
            c.border = juce::Colour(0xFF2A2A3A);
            c.tracker = juce::Colour(0xFF3A3A4A);
            break;
        case ColourScheme::Light:
            c.bg = juce::Colour(0xFFF8F8FA);
            c.bgAlt = juce::Colour(0xFFE8E8EE);
            c.accent = juce::Colour(0xFF00A888);
            c.accentAlt = juce::Colour(0xFF5A4AD8);
            c.text = juce::Colour(0xFF202020);
            c.textDim = juce::Colour(0xFF6A6A7A);
            c.border = juce::Colour(0xFFD0D0DA);
            c.tracker = juce::Colour(0xFFE0E0EA);
            break;
        case ColourScheme::HighContrast:
            c.bg = juce::Colour(0xFF000000);
            c.bgAlt = juce::Colour(0xFF111111);
            c.accent = juce::Colour(0xFF00FF88);
            c.accentAlt = juce::Colour(0xFFFF00AA);
            c.text = juce::Colour(0xFFFFFFFF);
            c.textDim = juce::Colour(0xFFAAAAAA);
            c.border = juce::Colour(0xFF333333);
            c.tracker = juce::Colour(0xFF222222);
            break;
    }
    return c;
}

void ModernLookAndFeel::drawModernKnob(juce::Graphics& g, int cx, int cy, int radius, float pos, const Colours& c)
{
    float angle = -juce::MathConstants<float>::pi * 0.75f + pos * juce::MathConstants<float>::pi * 1.5f;
    
    // Background track
    g.setColour(c.tracker);
    juce::Path track;
    track.addCentredArc(cx, cy, radius, radius, 0, -juce::MathConstants<float>::pi * 0.75f, juce::MathConstants<float>::pi * 0.75f, true);
    g.strokePath(track, juce::PathStrokeType(3.0f));
    
    // Active track
    g.setColour(c.accent);
    juce::Path activeTrack;
    float endAngle = -juce::MathConstants<float>::pi * 0.75f + pos * juce::MathConstants<float>::pi * 1.5f;
    activeTrack.addCentredArc(cx, cy, radius, radius, 0, -juce::MathConstants<float>::pi * 0.75f, endAngle, true);
    g.strokePath(activeTrack, juce::PathStrokeType(3.0f));
    
    // Knob center
    g.setColour(c.bgAlt);
    g.fillEllipse(cx - radius + 3, cy - radius + 3, (radius - 3) * 2, (radius - 3) * 2);
    g.setColour(c.border);
    g.drawEllipse(cx - radius + 3, cy - radius + 3, (radius - 3) * 2, (radius - 3) * 2, 1.0f);
    
    // Indicator dot
    float dx = std::cos(angle) * (radius - 6);
    float dy = std::sin(angle) * (radius - 6);
    g.setColour(c.accent);
    g.fillEllipse(cx + dx - 4, cy + dy - 4, 8, 8);
}

void ModernLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPosProportional, float rotaryStartAngle,
                                         float rotaryEndAngle, juce::Slider& slider)
{
    int size = juce::jmin(width, height);
    int cx = x + width / 2;
    int cy = y + height / 2;
    int radius = size / 2 - 4;
    
    drawModernKnob(g, cx, cy, radius, sliderPosProportional, colours);
    
    // Value display
    if (slider.isMouseOverOrDragging() || slider.isFocused()) {
        g.setColour(colours.text);
        g.setFont(juce::Font(modernFont).withHeight(12.0f));
        juce::String val = slider.getTextFromValue(slider.getValue());
        g.drawText(val, x, y + height + 4, width, 20, juce::Justification::centred, true);
    }
}

void ModernLookAndFeel::drawEnvelope(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                                     float attack, float decay, float sustain, float release)
{
    g.setColour(colours.bgAlt);
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    
    g.setColour(colours.border);
    g.drawRect(bounds, 1.0f);
    
    // Grid
    g.setColour(colours.border.withAlpha(0.3f));
    for (int i = 1; i < 4; ++i) {
        int x = bounds.getX() + bounds.getWidth() * i / 4;
        g.drawVerticalLine(x, bounds.getY(), bounds.getBottom());
        int y = bounds.getY() + bounds.getHeight() * i / 4;
        g.drawHorizontalLine(y, bounds.getX(), bounds.getRight());
    }
    
    // Envelope curve
    g.setColour(colours.accent);
    juce::Path env;
    float totalTime = attack + decay + 0.5f + release; // Normalized
    if (totalTime <= 0) totalTime = 1.0f;
    
    float x0 = bounds.getX();
    float y0 = bounds.getBottom();
    
    env.startNewSubPath(x0, y0);
    
    // Attack
    float x1 = x0 + bounds.getWidth() * (attack / totalTime);
    float y1 = bounds.getY();
    env.lineTo(x1, y1);
    
    // Decay
    float x2 = x1 + bounds.getWidth() * (decay / totalTime);
    float y2 = bounds.getBottom() - (bounds.getHeight() * sustain);
    env.lineTo(x2, y2);
    
    // Sustain
    float x3 = x2 + bounds.getWidth() * 0.5f / totalTime;
    env.lineTo(x3, y2);
    
    // Release
    float x4 = bounds.getRight();
    env.lineTo(x4, y0);
    
    g.strokePath(env, juce::PathStrokeType(2.0f));
}

void ModernLookAndFeel::drawFilterResponse(juce::Graphics& g, const juce::Rectangle<int>& bounds,
                                           float cutoff, float resonance, int type)
{
    g.setColour(colours.bgAlt);
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    g.setColour(colours.border);
    g.drawRect(bounds, 1.0f);
    
    // Frequency grid (log scale)
    g.setColour(colours.border.withAlpha(0.3f));
    float freqs[] = {20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
    for (float f : freqs) {
        float x = bounds.getX() + bounds.getWidth() * (std::log10(f/20.0) / std::log10(1000.0));
        if (x >= bounds.getX() && x <= bounds.getRight())
            g.drawVerticalLine(int(x), bounds.getY(), bounds.getBottom());
    }
    
    // Gain grid
    for (int i = 1; i < 4; ++i) {
        int y = bounds.getY() + bounds.getHeight() * i / 4;
        g.drawHorizontalLine(y, bounds.getX(), bounds.getRight());
    }
    
    // Response curve
    g.setColour(colours.accent);
    juce::Path curve;
    bool first = true;
    
    for (int x = bounds.getX(); x <= bounds.getRight(); x += 2) {
        float normX = float(x - bounds.getX()) / bounds.getWidth();
        float freq = std::pow(10.0f, normX * 3.0f + std::log10(20.0f)); // 20 to 20000
        
        // Simple 24dB/oct filter response
        float gain = 0.0f;
        float fc = cutoff;
        float q = resonance * 10.0f + 0.707f;
        float ratio = freq / fc;
        
        switch (type) {
            case 0: // LP24
                gain = -24.0f * std::log10(std::sqrt(1.0f + std::pow(ratio, 4.0f)));
                break;
            case 1: // LP12
                gain = -12.0f * std::log10(std::sqrt(1.0f + std::pow(ratio, 2.0f)));
                break;
            case 2: // HP24
                gain = -24.0f * std::log10(std::sqrt(1.0f + std::pow(1.0f/ratio, 4.0f)));
                break;
            case 3: // HP12
                gain = -12.0f * std::log10(std::sqrt(1.0f + std::pow(1.0f/ratio, 2.0f)));
                break;
            default:
                gain = 0.0f;
        }
        
        // Resonance peak
        if (type <= 1 && ratio > 0.9f && ratio < 1.1f && resonance > 0.1f) {
            gain += resonance * 12.0f * (1.0f - std::abs(ratio - 1.0f) * 10.0f);
        }
        
        // Clamp
        gain = juce::jlimit(-60.0f, 12.0f, gain);
        
        float y = bounds.getBottom() - bounds.getHeight() * (gain + 60.0f) / 72.0f;
        
        if (first) {
            curve.startNewSubPath(x, y);
            first = false;
        } else {
            curve.lineTo(x, y);
        }
    }
    
    g.strokePath(curve, juce::PathStrokeType(2.0f));
}

void ModernLookAndFeel::drawLFOWaveform(juce::Graphics& g, const juce::Rectangle<int>& bounds, int waveform)
{
    g.setColour(colours.bgAlt);
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    g.setColour(colours.border);
    g.drawRect(bounds, 1.0f);
    
    // Center line
    g.setColour(colours.border.withAlpha(0.3f));
    g.drawHorizontalLine(bounds.getCentreY(), bounds.getX(), bounds.getRight());
    
    g.setColour(colours.accent);
    juce::Path wave;
    wave.startNewSubPath(bounds.getX(), bounds.getCentreY());
    
    int steps = bounds.getWidth();
    for (int i = 0; i <= steps; ++i) {
        float t = float(i) / steps;
        float y = 0.0f;
        
        switch (waveform) {
            case 0: // Sine
                y = std::sin(t * 2.0f * juce::MathConstants<float>::pi * 2.0f); break;
            case 1: // Triangle
                y = 2.0f * std::abs(2.0f * (t * 2.0f - std::floor(t * 2.0f + 0.5f))) - 1.0f; break;
            case 2: // Saw
                y = 2.0f * (t * 2.0f - std::floor(t * 2.0f + 0.5f)); break;
            case 3: // Square
                y = (std::sin(t * 2.0f * juce::MathConstants<float>::pi * 2.0f) > 0) ? 1.0f : -1.0f; break;
            case 4: // S&H
                y = (int(t * 8.0f) % 2 == 0) ? 1.0f : -1.0f; break;
            case 5: // Random - draw as noise
                y = (int(t * 100.0f) % 2 == 0) ? 1.0f : -1.0f; break;
        }
        
        float x = bounds.getX() + t * bounds.getWidth();
        float cy = bounds.getCentreY() - y * (bounds.getHeight() * 0.4f);
        
        if (i == 0) wave.startNewSubPath(x, cy);
        else wave.lineTo(x, cy);
    }
    
    g.strokePath(wave, juce::PathStrokeType(2.0f));
}

juce::Font ModernLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(modernFont).withHeight(11.0f);
}

juce::Font ModernLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return juce::Font(modernFont).withHeight(12.0f);
}

void ModernLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                     int buttonX, int buttonY, int buttonW, int buttonH,
                                     juce::ComboBox& box)
{
    juce::Rectangle<int> bounds(0, 0, width, height);
    
    g.setColour(isButtonDown ? colours.bgAlt : colours.bg);
    g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    
    g.setColour(colours.border);
    g.drawRoundedRectangle(bounds.toFloat(), 4.0f, 1.0f);
    
    // Arrow
    g.setColour(colours.accent);
    float arrowSize = height * 0.25f;
    float cx = width - buttonW + buttonW * 0.5f;
    float cy = height * 0.5f;
    juce::Path arrow;
    arrow.addTriangle(cx - arrowSize/2, cy - arrowSize/3,
                      cx + arrowSize/2, cy - arrowSize/3,
                      cx, cy + arrowSize/3);
    g.fillPath(arrow);
}

void ModernLookAndFeel::drawTooltip(juce::Graphics& g, const juce::String& text, int w, int h)
{
    g.setColour(colours.bg.darker(0.1f));
    g.fillRoundedRectangle(0, 0, w, h, 4.0f);
    g.setColour(colours.border);
    g.drawRoundedRectangle(0.5f, 0.5f, w - 1, h - 1, 4.0f, 1.0f);
    g.setColour(colours.text);
    g.setFont(juce::Font(modernFont).withHeight(11.0f));
    g.drawText(text, 8, 0, w - 16, h, juce::Justification::centred, true);
}