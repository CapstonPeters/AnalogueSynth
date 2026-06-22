#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class AnalogSynthAudioProcessor;

/*
 * Design principles (Codex/Serum-inspired):
 *   - Dark uniform background (#0D0D14)
 *   - Rounded panel cards with subtle gradient fills
 *   - Accent-colored section headers (cyan=OSC, orange=Filter, amber=Env, green=LFO, violet=Mod)
 *   - 42px knobs with white dot indicators, arc fill from 7-o'clock to 5-o'clock
 *   - Compact combo boxes with drop-shadow arrows
 *   - 8px grid for spacing — everything aligns to it
 */

//==============================================================================
// Forward declarations
struct KnobGroup;
class SectionPanel;
class WaveformPreview;

//==============================================================================
class AnalogSynthAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AnalogSynthAudioProcessorEditor (AnalogSynthAudioProcessor&);
    ~AnalogSynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // -----------------------------------------------------------------
    // Embedded classes (complete types needed for std::unique_ptr)
    // -----------------------------------------------------------------

    class SynthLookAndFeel;
    std::unique_ptr<SynthLookAndFeel> laf;

    struct KnobGroup
    {
        juce::Slider slider;
        juce::Label  label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;

        void setup (const juce::String& paramID, juce::AudioProcessorValueTreeState&,
                    float min, float max, float step, float def,
                    const juce::String& txt, const juce::String& suffix,
                    juce::Component* parent, juce::LookAndFeel* lf);
    };

    class SectionPanel : public juce::Component
    {
    public:
        SectionPanel (const juce::String& title, juce::Colour accent);
        void paint (juce::Graphics&) override;
    private:
        juce::String title;
        juce::Colour  accent;
    };

    class WaveformPreview : public juce::Component
    {
    public:
        void paint (juce::Graphics&) override;
        void setType (const juce::String& t) { waveType = t; repaint(); }
    private:
        juce::String waveType = "Saw";
    };

    class EnvDisplay : public juce::Component
    {
    public:
        void paint (juce::Graphics&) override;
        void setParams (float att, float dec, float sus, float rel);
    private:
        float attack  = 0.01f;
        float decay   = 0.3f;
        float sustain = 0.7f;
        float release = 0.5f;
    };

    class FilterCurveDisplay : public juce::Component
    {
    public:
        void paint (juce::Graphics&) override;
        void setParams (int type, float cutoff, float reso);
    private:
        int   filterType   = 0;
        float cutoffFreq   = 1000.0f;
        float resonance    = 0.0f;
    };

    // -----------------------------------------------------------------
    // State
    // -----------------------------------------------------------------
    AnalogSynthAudioProcessor& proc;
    juce::AudioProcessorValueTreeState& apvts;

    bool built = false;
    void buildUI();

    // -----------------------------------------------------------------
    // Global (top bar)
    // -----------------------------------------------------------------
    KnobGroup masterGain, polyphony, pitchBend;

    // -----------------------------------------------------------------
    // Oscillators × 3
    // -----------------------------------------------------------------
    KnobGroup osc1Lev, osc1Pit, osc1Fin, osc1Pan, osc1Uni, osc1Det, osc1PW, osc1Scn;
    KnobGroup osc2Lev, osc2Pit, osc2Fin, osc2Pan, osc2Uni, osc2Det, osc2PW, osc2Scn;
    KnobGroup osc3Lev, osc3Pit, osc3Fin, osc3Pan, osc3Uni, osc3Det, osc3PW, osc3Scn;
    KnobGroup subLev, subPit, noiseLev;

    juce::ComboBox osc1Wave, osc2Wave, osc3Wave, subWave, noiseType;
    juce::ComboBox osc1WT, osc2WT, osc3WT;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        osc1WaveA, osc2WaveA, osc3WaveA, subWaveA, noiseTypeA,
        osc1WTA, osc2WTA, osc3WTA;

    WaveformPreview wf1, wf2, wf3;

    SectionPanel oscPanel {"OSCILLATORS",      juce::Colour(0xFF00E5B0)};

    // -----------------------------------------------------------------
    // Filter
    // -----------------------------------------------------------------
    KnobGroup filtCut, filtRes, filtDrv, filtKey, filtVel;
    juce::ComboBox    filtType;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filtTypeA;
    FilterCurveDisplay filterCurve;
    SectionPanel filtPanel {"FILTER", juce::Colour(0xFFFF7744)};

    // -----------------------------------------------------------------
    // Envelopes (Amp + Filter)
    // -----------------------------------------------------------------
    KnobGroup ampA, ampD, ampS, ampR, ampVel;
    KnobGroup fenvA, fenvD, fenvS, fenvR, fenvAmt, fenvVel;
    SectionPanel ampPanel  {"AMP ENVELOPE",    juce::Colour(0xFF88BBFF)};
    SectionPanel fenvPanel {"FILTER ENVELOPE", juce::Colour(0xFFBB88FF)};
    EnvDisplay ampCurve, fenvCurve;

    // -----------------------------------------------------------------
    // LFOs (× 2)
    // -----------------------------------------------------------------
    KnobGroup lfo1Rate, lfo1Amt, lfo1Del, lfo1Fade;
    KnobGroup lfo2Rate, lfo2Amt, lfo2Del, lfo2Fade;
    juce::ComboBox lfo1Wave, lfo2Wave;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfo1WaveA, lfo2WaveA;
    SectionPanel lfoPanel {"LFOs", juce::Colour(0xFFFFAA55)};

    // -----------------------------------------------------------------
    // Mod Matrix
    // -----------------------------------------------------------------
    SectionPanel modPanel {"MOD MATRIX", juce::Colour(0xFF8888AA)};
    juce::Label modLabel;

    // -----------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------
    static constexpr int kKnob    = 42;
    static constexpr int kLabelH  = 14;
    static constexpr int kComboH  = 24;
    static constexpr int kGap     = 8;
    static constexpr int kPanelPad = 32;   // top padding inside a panel (title bar)
    static constexpr int kInset   = 4;     // inner padding

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogSynthAudioProcessorEditor)
};
