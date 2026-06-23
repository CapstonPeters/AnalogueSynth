#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class AnalogSynthAudioProcessor;

/*
 *  ╔═══════════════════════════════════════════╗
 *  ║  AnalogueSynth  —  Premium Dark UI        ║
 *  ║  Design System v2.0                       ║
 *  ╠═══════════════════════════════════════════╣
 *  ║  Signal Path:  Cyan     #00E5B0           ║
 *  ║  Modulation:   Amber    #FFAA55           ║
 *  ║  Effects:      Purple   #7C4DFF           ║
 *  ║  Background:   #080810                    ║
 *  ║  Surface:      #12121E                    ║
 *  ║  Panel:        #1A1A2E                    ║
 *  ╚═══════════════════════════════════════════╝
 *
 *  Architecture:
 *    • Left → Right signal flow philosophy
 *    • Modular rack-style panels with gradient headers
 *    • Animated signal routing diagram
 *    • Collapsible step sequencer (Custom arp mode)
 *    • Expansion-ready: empty slots for future modules
 */

//==============================================================================
// Design tokens
namespace ColourID
{
    // Functional
    inline constexpr auto kSignalPath  = 0xFF00E5B0;  // cyan — osc, filter, amp
    inline constexpr auto kModulation  = 0xFFFFAA55;  // amber — lfo, env, mod matrix
    inline constexpr auto kEffects     = 0xFF7C4DFF;  // purple — fx
    inline constexpr auto kArp         = 0xFF26A69A;  // teal — arp/seq (secondary signal)

    // Neutrals
    inline constexpr auto kBg          = 0xFF080810;  // deepest background
    inline constexpr auto kSurface     = 0xFF12121E;  // panel card base
    inline constexpr auto kPanel       = 0xFF1A1A2E;  // elevated surface
    inline constexpr auto kBorder      = 0xFF2A2A3E;  // subtle borders
    inline constexpr auto kTextPrimary = 0xFFCCCCDD;
    inline constexpr auto kTextMuted   = 0xFF66667A;
    inline constexpr auto kTextDim     = 0xFF44445A;
}

//==============================================================================
// Forward declarations
struct KnobGroup;
class SectionPanel;
class WaveformPreview;
class SignalFlowDiagram;

//==============================================================================
class AnalogSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    AnalogSynthAudioProcessorEditor (AnalogSynthAudioProcessor&);
    ~AnalogSynthAudioProcessorEditor() override;

    void paint  (juce::Graphics&) override;
    void resized() override;

private:
    // -----------------------------------------------------------------
    // Embedded classes
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
                    juce::Component* parent, juce::LookAndFeel* lf,
                    juce::Colour accent = juce::Colour(ColourID::kSignalPath));
    };

    // --- Rounded panel card with gradient header ---
    class SectionPanel : public juce::Component
    {
    public:
        SectionPanel (const juce::String& title, juce::Colour accent);
        void paint (juce::Graphics&) override;
    private:
        juce::String title;
        juce::Colour  accent;
    };

    // --- Mini oscilloscope per oscillator ---
    class WaveformPreview : public juce::Component
    {
    public:
        void paint (juce::Graphics&) override;
        void setType (const juce::String& t) { waveType = t; repaint(); }
    private:
        juce::String waveType = "Saw";
    };

    // --- Animated ADSR envelope curve ---
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

    // --- Filter response curve ---
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

    // --- Signal flow routing diagram ---
    class SignalFlowDiagram : public juce::Component
    {
    public:
        void paint (juce::Graphics&) override;
    };

    // -----------------------------------------------------------------
    // State
    // -----------------------------------------------------------------
    AnalogSynthAudioProcessor& proc;
    juce::AudioProcessorValueTreeState& apvts;
    bool built = false;
    void buildUI();

    // Scroll support — content is taller than window, user scrolls vertically
    juce::Viewport viewport;
    juce::Component contentComp;

    // -----------------------------------------------------------------
    // Global (top bar)
    // -----------------------------------------------------------------
    KnobGroup masterGain, polyphony, pitchBend;

    // Preset browser
    juce::ComboBox presetBox;
    void loadPreset(int idx);
    void initPresets();

    // -----------------------------------------------------------------
    // Signal flow diagram
    // -----------------------------------------------------------------
    SignalFlowDiagram signalFlow;

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
    SectionPanel oscPanel {"OSCILLATORS", juce::Colour(ColourID::kSignalPath)};

    // Sync + FM toggles
    juce::TextButton osc2SyncToggle, osc3FMToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> osc2SyncToggleA, osc3FMToggleA;

    // -----------------------------------------------------------------
    // Filter
    // -----------------------------------------------------------------
    KnobGroup filtCut, filtRes, filtDrv, filtKey, filtVel;
    juce::ComboBox filtType;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filtTypeA;
    FilterCurveDisplay filterCurve;
    SectionPanel filtPanel {"FILTER", juce::Colour(ColourID::kSignalPath)};

    // -----------------------------------------------------------------
    // Envelopes
    // -----------------------------------------------------------------
    KnobGroup ampA, ampD, ampS, ampR, ampVel;
    KnobGroup fenvA, fenvD, fenvS, fenvR, fenvAmt, fenvVel;
    SectionPanel ampPanel  {"AMP ENVELOPE",    juce::Colour(ColourID::kModulation)};
    SectionPanel fenvPanel {"FILTER ENVELOPE", juce::Colour(ColourID::kModulation)};
    EnvDisplay ampCurve, fenvCurve;

    // -----------------------------------------------------------------
    // LFOs (3)
    // -----------------------------------------------------------------
    KnobGroup lfo1Rate, lfo1Amt, lfo1Del, lfo1Fade;
    KnobGroup lfo2Rate, lfo2Amt, lfo2Del, lfo2Fade;
    KnobGroup lfo3Rate, lfo3Amt, lfo3Del, lfo3Fade;
    juce::ComboBox lfo1Wave, lfo2Wave, lfo3Wave;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfo1WaveA, lfo2WaveA, lfo3WaveA;
    SectionPanel lfoPanel {"LFOs", juce::Colour(ColourID::kModulation)};

    // -----------------------------------------------------------------
    // Arpeggiator
    // -----------------------------------------------------------------
    juce::ComboBox arpMode, arpRate;
    KnobGroup arpOctaves, arpGate, arpSteps, arpSwing;
    juce::TextButton arpToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> arpModeA, arpRateA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> arpToggleA;
    SectionPanel arpPanel {"ARPEGGIATOR", juce::Colour(ColourID::kArp)};

    // Step sequencer grid (visible in Custom mode)
    SectionPanel stepPanel {"STEP SEQUENCER", juce::Colour(ColourID::kArp)};
    juce::Slider arpStepSliders[16];
    juce::ToggleButton arpStepToggles[16];
    juce::Label arpStepLabels[16];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> arpStepSliderA[16];
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> arpStepToggleA[16];

    // -----------------------------------------------------------------
    // FX
    // -----------------------------------------------------------------
    SectionPanel fxPanel {"EFFECTS", juce::Colour(ColourID::kEffects)};

    // Chorus
    KnobGroup chRate, chDepth, chMix;
    juce::TextButton chToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> chToggleA;
    // Flanger
    KnobGroup flRate, flDepth, flFb, flMix;
    juce::TextButton flToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> flToggleA;
    // Phaser
    KnobGroup phRate, phDepth, phFb, phMix;
    juce::TextButton phToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> phToggleA;
    // Delay
    KnobGroup dlyTimeL, dlyTimeR, dlyFb2, dlyWet2;
    juce::TextButton dlyToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> dlyToggleA;
    // Reverb
    KnobGroup revSize2, revDamp, revWet2;
    juce::TextButton revToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> revToggleA;
    juce::Label chLabel, flLabel, phLabel, dlyLabel2, revLabel2;

    // -----------------------------------------------------------------
    // Macro
    // -----------------------------------------------------------------
    KnobGroup macro1, macro2, macro3, macro4;
    SectionPanel macroPanel {"MACRO", juce::Colour(ColourID::kModulation)};

    // -----------------------------------------------------------------
    // Mod Matrix
    // -----------------------------------------------------------------
    SectionPanel modPanel {"MOD MATRIX", juce::Colour(ColourID::kModulation)};
    juce::Label modLabel;

    // -----------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------
    static constexpr int kKnob    = 48;   // increased from 42
    static constexpr int kLabelH  = 14;
    static constexpr int kComboH  = 24;
    static constexpr int kGap     = 10;
    static constexpr int kPanelPad = 36;  // increased from 32 for gradient headers
    static constexpr int kInset   = 6;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalogSynthAudioProcessorEditor)
};
