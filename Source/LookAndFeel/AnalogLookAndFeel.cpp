#include "AnalogLookAndFeel.h"

AnalogLookAndFeel::AnalogLookAndFeel()
{
    // Load vintage font (fallback to system)
    vintageFont = juce::Typeface::createSystemTypefaceFor(
        juce::BinaryData::getNamedResource("vintage_font", nullptr));
    if (!vintageFont) {
        vintageFont = juce::Font("Courier New", 12.0f, juce::Font::bold).getTypeface();
    }
    
    setColourScheme(ColourScheme::Vintage);
    createKnobStrip();
}

void AnalogLookAndFeel::setColourScheme(ColourScheme scheme)
{
    currentScheme = scheme;
    colours = getSchemeColours(scheme);
    createKnobStrip();
}

void AnalogLookAndFeel::createKnobStrip()
{
    // Create a 64-frame knob strip for smooth animation
    const int frames = 64;
    const int size = 64;
    knobStrip = juce::Image(juce::Image::ARGB, size, size * frames, true);
    
    juce::Graphics g(knobStrip);
    for (int frame = 0; frame < frames; ++frame) {
        float pos = float(frame) / float(frames - 1);
        float angle = -1.25f * juce::MathConstants<float>::pi + pos * 2.5f * juce::MathConstants<float>::pi;
        
        // Knob body
        g.setColour(colours.knobBody);
        g.fillEllipse(4, 4 + frame * size, size - 8, size - 8);
        
        // Knob body highlight/shadow
        g.setColour(colours.knobBody.brighter(0.1f));
        g.drawEllipse(4, 4 + frame * size, size - 8, size - 8, 1.0f);
        g.setColour(colours.knobBody.darker(0.2f));
        g.drawEllipse(5, 5 + frame * size, size - 10, size - 10, 0.5f);
        
        // Pointer
        float px = size/2 + std::cos(angle) * (size * 0.3f);
        float py = size/2 + std::sin(angle) * (size * 0.3f);
        g.setColour(colours.knobPointer);
        g.drawLine(size/2, size/2 + frame * size, px, py + frame * size, 2.5f);
        
        // Center dot
        g.setColour(colours.knobPointer.darker(0.3f));
        g.fillEllipse(size/2 - 3, size/2 - 3 + frame * size, 6, 6);
    }
}

AnalogLookAndFeel::Colours AnalogLookAndFeel::getSchemeColours(ColourScheme scheme)
{
    Colours c;
    switch (scheme) {
        case ColourScheme::Vintage:
            c.bg = juce::Colour(0xFF1A1A1A);
            c.knobBody = juce::Colour(0xFF3D3D3D);
            c.knobPointer = juce::Colour(0xFFE8C56D); // Gold
            c.knobTracker = juce::Colour(0xFF4A4A2A);
            c.text = juce::Colour(0xFFF0E0C0);
            c.textDim = juce::Colour(0xFF8A8A6A);
            c.border = juce::Colour(0xFF2A2A2A);
            break;
        case ColourScheme::Aged:
            c.bg = juce::Colour(0xFF1E1A16);
            c.knobBody = juce::Colour(0xFF4A4238);
            c.knobPointer = juce::Colour(0xFFD4A843);
            c.knobTracker = juce::Colour(0xFF5A4A3A);
            c.text = juce::Colour(0xFFE8D8C0);
            c.textDim = juce::Colour(0xFF9A8A7A);
            c.border = juce::Colour(0xFF2E2A26);
            break;
        case ColourScheme::Dark:
            c.bg = juce::Colour(0xFF121212);
            c.knobBody = juce::Colour(0xFF2A2A2A);
            c.knobPointer = juce::Colour(0xFFCCAA44);
            c.knobTracker = juce::Colour(0xFF3A3A2A);
            c.text = juce::Colour(0xFFE0E0E0);
            c.textDim = juce::Colour(0xFF808080);
            c.border = juce::Colour(0xFF1A1A1A);
            break;
    }
    return c;
}

void AnalogLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPosProportional, float rotaryStartAngle,
                                         float rotaryEndAngle, juce::Slider& slider)
{
    int size = juce::jmin(width, height);
    int cx = x + width / 2;
    int cy = y + height / 2;
    int radius = size / 2 - 4;
    
    // Draw from knob strip
    int frame = juce::jlimit(0, 63, int(sliderPosProportional * 63.0f));
    juce::Rectangle<int> srcRect(0, frame * 64, 64, 64);
    juce::Rectangle<int> destRect(cx - size/2, cy - size/2, size, size);
    g.drawImage(knobStrip, destRect.toFloat(), srcRect.toFloat());
    
    // Value display below knob
    if (slider.isMouseOverOrDragging() || slider.isFocused()) {
        g.setColour(colours.text);
        g.setFont(11.0f);
        juce::String val = slider.getTextFromValue(slider.getValue());
        g.drawText(val, x, y + height + 2, width, 20, juce::Justification::centred, true);
    }
}

juce::Font AnalogLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(vintageFont).withHeight(12.0f);
}

juce::Font AnalogLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return juce::Font(vintageFont).withHeight(13.0f);
}

void AnalogLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                     int buttonX, int buttonY, int buttonW, int buttonH,
                                     juce::ComboBox& box)
{
    juce::Rectangle<int> boxBounds(0, 0, width, height);
    
    g.setColour(colours.bg.darker(0.2f));
    g.fillRoundedRectangle(boxBounds.toFloat(), 4.0f);
    
    g.setColour(isButtonDown ? colours.knobPointer : colours.border);
    g.drawRoundedRectangle(boxBounds.toFloat(), 4.0f, 1.5f);
    
    // Arrow
    g.setColour(colours.text);
    float arrowSize = height * 0.3f;
    float cx = width - buttonW + buttonW * 0.5f;
    float cy = height * 0.5f;
    juce::Path arrow;
    arrow.addTriangle(cx - arrowSize/2, cy - arrowSize/3,
                      cx + arrowSize/2, cy - arrowSize/3,
                      cx, cy + arrowSize/3);
    g.fillPath(arrow);
}

void AnalogLookAndFeel::drawTooltip(juce::Graphics& g, const juce::String& text, int w, int h)
{
    g.setColour(colours.bg.brighter(0.1f));
    g.fillRoundedRectangle(0, 0, w, h, 6.0f);
    g.setColour(colours.border);
    g.drawRoundedRectangle(0.5f, 0.5f, w - 1, h - 1, 6.0f, 1.0f);
    g.setColour(colours.text);
    g.setFont(11.0f);
    g.drawText(text, 8, 0, w - 16, h, juce::Justification::centred, true);
}