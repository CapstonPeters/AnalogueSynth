#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class AnalogSynthAudioProcessor;

// Forward declarations to reduce template instantiation in header
struct KnobGroup;
class SectionPanel;

class AnalogSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AnalogSynthAudioProcessorEditor (AnalogSynthAudioProcessor&);
    ~AnalogSynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class SynthLookAndFeel;
    std::unique_ptr<SynthLookAndFeel> lookAndFeel;

    // Complete type definitions for unique_ptr
    struct KnobGroup
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

        void setup(const juce::String& paramID, juce::AudioProcessorValueTreeState& apvts,
                   float min, float max, float interval, float def,
                   const juce::String& labelText, const juce::String& suffix,
                   juce::Component* parent, juce::LookAndFeel* laf);

        void setBounds(juce::Rectangle<int> area, int knobSize, int labelHeight);
    };

    class SectionPanel : public juce::Component
    {
    public:
        SectionPanel(const juce::String& title, juce::Colour accentColour);
        void paint(juce::Graphics& g) override;

    private:
        juce::String titleText;
        juce::Colour accent;
    };

    class WaveformPreview : public juce::Component
    {
    public:
        WaveformPreview() = default;
        void paint(juce::Graphics& g) override;
        void setWaveType(const juce::String& type) { waveType = type; repaint(); }
    private:
        juce::String waveType = "Saw";
    };

    AnalogSynthAudioProcessor& processorRef;
    juce::AudioProcessorValueTreeState& apvts;

    // Deferred UI initialization — built on first resized() to avoid
    // Windows static initialization / thread-initialization crashes
    bool initialized = false;
    void buildUI();

    // Header controls
    juce::TextButton testToneButton;
    juce::ComboBox waveTypeComboBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveTypeAttachment;

    // Waveform previews (oscilloscope-style)
    std::unique_ptr<WaveformPreview> wf1, wf2, wf3;

    // Knob groups
    std::unique_ptr<KnobGroup> masterGainKnob, polyphonyKnob, pitchBendKnob;

    std::unique_ptr<KnobGroup> osc1Level, osc1Pitch, osc1Fine, osc1Pan, osc1Unison, osc1Detune, osc1PW, osc1Scan;
    std::unique_ptr<KnobGroup> osc2Level, osc2Pitch, osc2Fine, osc2Pan, osc2Unison, osc2Detune, osc2PW, osc2Scan;
    std::unique_ptr<KnobGroup> osc3Level, osc3Pitch, osc3Fine, osc3Pan, osc3Unison, osc3Detune, osc3PW, osc3Scan;
    std::unique_ptr<KnobGroup> subLevel, subPitch, noiseLevel;

    std::unique_ptr<KnobGroup> filterCutoff, filterReso, filterDrive, filterKeyTrk, filterVelTrk;

    std::unique_ptr<KnobGroup> ampAtt, ampDec, ampSus, ampRel, ampVel;
    std::unique_ptr<KnobGroup> filtAtt, filtDec, filtSus, filtRel, filtAmt, filtVel;

    std::unique_ptr<KnobGroup> lfo1Rate, lfo1Amt, lfo1Delay, lfo1Fade;
    std::unique_ptr<KnobGroup> lfo2Rate, lfo2Amt, lfo2Delay, lfo2Fade;

    // Wave combos
    juce::ComboBox osc1Wave, osc2Wave, osc3Wave, subWave, noiseWave, filterType, lfo1Wave, lfo2Wave;
    juce::ComboBox osc1Wavetable, osc2Wavetable, osc3Wavetable;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        osc1WaveAtt, osc2WaveAtt, osc3WaveAtt, subWaveAtt, noiseWaveAtt,
        filterTypeAtt, lfo1WaveAtt, lfo2WaveAtt,
        osc1WavetableAtt, osc2WavetableAtt, osc3WavetableAtt;

    // Section panels
    std::unique_ptr<SectionPanel> oscPanel, filterPanel, ampEnvPanel;
    std::unique_ptr<SectionPanel> filtEnvPanel, lfoPanel, modPanel;

    juce::Label modLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogSynthAudioProcessorEditor)
};
