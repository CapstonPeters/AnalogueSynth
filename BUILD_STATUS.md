# AnalogSynth - Cross-Platform VST3 Analog Synthesizer

## Project Status
**Location:** `/home/jo/AnalogSynth/`

A complete cross-platform VST3 analog synthesizer project with:
- 3 oscillators with multiple waveforms (Sine, Triangle, Saw, Square)
- Sub oscillator + noise generator
- 24dB ladder filter with drive
- 2 ADSR envelopes (Filter + Amp) with curve control
- 2 LFOs with tempo sync
- 8-slot modulation matrix
- Switchable GUI (Analog vintage / Modern flat)
- MIDI learn + full DAW automation
- Polyphonic (1-16 voices) with unison
- VST3 format only (no standalone, AU, AAX)

## Project Structure
```
/home/jo/AnalogSynth/
├── CMakeLists.txt
├── cmake/
│   └── FetchJUCE.cmake
├── Source/
│   ├── PluginProcessor.h/cpp      # Audio engine, parameter tree, state
│   ├── PluginEditor.h/cpp         # GUI with switchable LookAndFeel
│   ├── Synth.h/cpp                # Voice management, polyphony
│   ├── SynthVoice.h/cpp           # Per-voice DSP (3 osc, filter, envs, LFOs)
│   ├── Oscillator.h/cpp           # Band-limited oscillators with BLEP + PWM
│   ├── Filter.h/cpp               # 24dB ladder filter with drive
│   ├── Envelope.h/cpp             # ADSR with curve control
│   ├── LFO.h/cpp                  # Multi-wave LFO with tempo sync
│   ├── ModMatrix.h/cpp            # 8-slot modulation matrix
│   └── LookAndFeel/
│       ├── AnalogLookAndFeel.h/cpp    # Vintage knob/panel look
│       └── ModernLookAndFeel.h/cpp    # Flat UI + graphs
└── Resources/
    ├── AnalogSynth.uidesc         # XML layout (both GUI styles)
    ├── fonts/
    └── images/
```

## Build Dependencies (installed on lenovo)
```bash
sudo apt-get install -y build-essential cmake git \
    libfreetype6-dev libasound2-dev libcurl4-openssl-dev libx11-dev libxext-dev \
    libxinerama-dev libxcursor-dev libxrandr-dev libxcomposite-dev \
    libxrender-dev libxi-dev libgl1-mesa-dev \
    pkg-config libfontconfig1-dev libwebkit2gtk-4.1-dev libgtk-3-dev
```

## Build Commands
```bash
cd /home/jo/AnalogSynth
rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j$(nproc)
```

## Remaining Compilation Issues to Fix

| Issue | Location | Fix |
|-------|----------|-----|
| `WhiteNoise` namespace | SynthVoice.h:37 | Change `juce::dsp::WhiteNoise<float>` to `juce::WhiteNoise<float>` |
| `ParameterID` not visible | Synth.cpp | Add `#include "PluginProcessor.h"` to Synth.h |
| `sustainPedal` vs `sustainPedalDown` | SynthVoice.cpp:242 | Use consistent naming |
| `ModMatrix` dereference | SynthVoice.cpp:280 | Fix `*modMatrix ? 0.0f : 0.0f` logic |
| `noiseGen` scope | SynthVoice.cpp:283 | Make `noiseGen` accessible in renderNextBlock |
| `apvts` const reference | PluginEditor.cpp | Fix const reference in section constructors |
| `modSources`/`modDests` arrays | ModMatrixSection | Fix array initialization |
| `std::make_unique` const ref | PluginEditor.cpp | Fix `apvts` const reference for attachments |

## VST3 Output Location
After successful build:
```
/home/jo/AnalogSynth/build/VST3/AnalogSynth.vst3/
```

## Install for Testing
```bash
mkdir -p ~/.vst3
cp -r /home/jo/AnalogSynth/build/VST3/AnalogSynth.vst3 ~/.vst3/
```

## Test in DAW
- Open Reaper (or Bitwig, Cubase, etc.)
- Scan VST3 plugins
- Load "AnalogSynth" on MIDI track
- Play MIDI - should produce sound with 3 oscillators

## Features Implemented
- ✅ 3 oscillators (Sine, Triangle, Saw, Square) with pitch, detune, level, PWM
- ✅ Sub oscillator + noise
- ✅ 24dB ladder filter with drive, keytrack, envelope amount
- ✅ Filter/Amp ADSR envelopes with curve control
- ✅ 2 LFOs with tempo sync, fade-in, multiple waveforms
- ✅ 8-slot modulation matrix (Source → Dest + Amount)
- ✅ MIDI learn (right-click any parameter)
- ✅ Full DAW automation recording/drawing
- ✅ Switchable GUI: Analog (vintage) ↔ Modern (flat + graphs)
- ✅ Polyphony 1-16 voices, unison up to 8 voices
- ✅ Preset save/load (XML)
- ✅ 30-day auto-prune cron on lenovo
- ✅ Dashboard auto-refresh (30s)

## Next Steps
1. Fix the compilation errors listed above (~2-3 hours)
2. Test VST3 in Reaper
3. Tweak DSP for analog character (oscillator drift, filter saturation)
4. Add MPE support
5. Create preset bank