#pragma once

// JUCE module includes
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

class ModernLookAndFeel  : public juce::LookAndFeel_V4
{
public:
    enum class ColourScheme { Dark, Light, HighContrast };
    
    ModernLookAndFeel();
    void setColourScheme(ColourScheme scheme);
    
    // Modern knob with value display
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;
    
    // ADSR envelope drawing
    void drawEnvelope(juce::Graphics&, const juce::Rectangle<int>&, 
                      float attack, float decay, float sustain, float release);
    
    // Filter response curve
    void drawFilterResponse(juce::Graphics&, const juce::Rectangle<int>&,
                            float cutoff, float resonance, int type);
    
    // LFO waveform display
    void drawLFOWaveform(juce::Graphics&, const juce::Rectangle<int>&, int waveform);
    
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
    ColourScheme currentScheme = ColourScheme::Dark;
    juce::Typeface::Ptr modernFont;
    
    struct Colours {
        juce::Colour bg, bgAlt, accent, accentAlt, text, textDim, border, tracker;
    };
    Colours colours;
    
    Colours getSchemeColours(ColourScheme scheme);
    void drawModernKnob(juce::Graphics&, int cx, int cy, int radius, float pos, const Colours&);
};