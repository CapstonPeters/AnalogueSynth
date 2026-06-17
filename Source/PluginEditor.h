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
    
        AnalogSynthAudioProcessor& processorRef;
        juce::AudioProcessorValueTreeState& apvts;
    
    // Top bar
    juce::TextButton testToneButton;
    juce::ComboBox waveTypeComboBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveTypeAttachment;
    
    // Knob groups
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
    
    // Wave combos
    juce::ComboBox osc1Wave, osc2Wave, osc3Wave, subWave, noiseWave, filterType, lfo1Wave, lfo2Wave;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> 
        osc1WaveAtt, osc2WaveAtt, osc3WaveAtt, subWaveAtt, noiseWaveAtt,
        filterTypeAtt, lfo1WaveAtt, lfo2WaveAtt;
    
    // Section panels
    SectionPanel oscPanel{"OSCILLATORS", juce::Colour(0xFF00D4AA)};
    SectionPanel filterPanel{"FILTER", juce::Colour(0xFFFF8844)};
    SectionPanel ampEnvPanel{"AMP ENVELOPE", juce::Colour(0xFF88AAFF)};
    SectionPanel filtEnvPanel{"FILTER ENVELOPE", juce::Colour(0xFFAA88FF)};
    SectionPanel lfoPanel{"LFOS", juce::Colour(0xFFFFAA88)};
    SectionPanel modPanel{"MODULATION", juce::Colour(0xFF888888)};
    
    juce::Label modLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogSynthAudioProcessorEditor)
};