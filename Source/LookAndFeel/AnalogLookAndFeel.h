#pragma once

// JUCE module includes
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

class AnalogLookAndFeel  : public juce::LookAndFeel_V4
{
public:
    enum class ColourScheme { Vintage, Aged, Dark };
    
    AnalogLookAndFeel();
    void setColourScheme(ColourScheme scheme);
    
    // Knob drawing
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;
    
    // Label fonts
    juce::Font getLabelFont(juce::Label&) override;
    
    // ComboBox
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox&) override;
    
    // Tooltip
    void drawTooltip(juce::Graphics&, const juce::String&, int w, int h) override;
    
private:
    ColourScheme currentScheme = ColourScheme::Vintage;
    juce::Image knobStrip;
    juce::Typeface::Ptr vintageFont;
    
    struct Colours {
        juce::Colour bg, knobBody, knobPointer, knobTracker, text, textDim, border;
    };
    Colours colours;
    
    void createKnobStrip();
    Colours getSchemeColours(ColourScheme scheme);
};