#include "SynthVoice.h"

SynthVoice::SynthVoice(ModMatrix* mm, int unisonV, float unisonD)
    : modMatrix(mm), unisonVoices(unisonV), unisonDetune(unisonD)
{
    unisonVoicesVec.resize(unisonVoices);
    for (int i = 0; i < unisonVoices; ++i) {
        float detune = (unisonVoices > 1) ? (float(i) / float(unisonVoices - 1) - 0.5f) * 2.0f * unisonDetune : 0.0f;
        unisonVoicesVec[i].detuneOffset = detune;
    }
}

bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<juce::SynthesiserSound*>(sound) != nullptr;
}

void SynthVoice::prepare (double sr, int samplesPerBlock)
{
    sampleRate = sr;
    
    for (auto& osc : oscillators) osc.prepare(sr);
    subOsc.prepare(sr);
    for (auto& oscVec : unisonVoicesVec) {
        for (auto& osc : oscVec.oscillators) osc.prepare(sr);
    }
    
    filter.prepare(sr);
    filterEnv.prepare(sr);
    ampEnv.prepare(sr);
    lfo1.prepare(sr);
    lfo2.prepare(sr);
    noiseGen.prepare(sr);
    
    for (auto& uv : unisonVoicesVec) {
        uv.filter.prepare(sr);
        uv.filterEnv.prepare(sr);
        uv.ampEnv.prepare(sr);
    }
}

void SynthVoice::release()
{
    for (auto& osc : oscillators) osc.reset();
    subOsc.reset();
    filter.reset();
    filterEnv.reset();
    ampEnv.reset();
    lfo1.reset();
    lfo2.reset();
    for (auto& uv : unisonVoicesVec) {
        for (auto& osc : uv.oscillators) osc.reset();
        uv.filter.reset();
        uv.filterEnv.reset();
        uv.ampEnv.reset();
    }
}

void SynthVoice::updateParameters(juce::AudioProcessorValueTreeState& apvts)
{
    auto& osc1Wave = *apvts.getRawParameterValue(ParameterID::osc1Wave);
    auto& osc1Pitch = *apvts.getRawParameterValue(ParameterID::osc1Pitch);
    auto& osc1Detune = *apvts.getRawParameterValue(ParameterID::osc1Detune);
    auto& osc1Level = *apvts.getRawParameterValue(ParameterID::osc1Level);
    auto& osc1PW = *apvts.getRawParameterValue(ParameterID::osc1PW);
    
    auto& osc2Wave = *apvts.getRawParameterValue(ParameterID::osc2Wave);
    auto& osc2Pitch = *apvts.getRawParameterValue(ParameterID::osc2Pitch);
    auto& osc2Detune = *apvts.getRawParameterValue(ParameterID::osc2Detune);
    auto& osc2Level = *apvts.getRawParameterValue(ParameterID::osc2Level);
    auto& osc2PW = *apvts.getRawParameterValue(ParameterID::osc2PW);
    auto& osc2Sync = *apvts.getRawParameterValue(ParameterID::osc2Sync);
    
    auto& osc3Wave = *apvts.getRawParameterValue(ParameterID::osc3Wave);
    auto& osc3Pitch = *apvts.getRawParameterValue(ParameterID::osc3Pitch);
    auto& osc3Detune = *apvts.getRawParameterValue(ParameterID::osc3Detune);
    auto& osc3Level = *apvts.getRawParameterValue(ParameterID::osc3Level);
    auto& osc3PW = *apvts.getRawParameterValue(ParameterID::osc3PW);
    
    auto& noiseLevel = *apvts.getRawParameterValue(ParameterID::noiseLevel);
    auto& subLevel = *apvts.getRawParameterValue(ParameterID::subLevel);
    
    auto& filterCutoff = *apvts.getRawParameterValue(ParameterID::filterCutoff);
    auto& filterRes = *apvts.getRawParameterValue(ParameterID::filterRes);
    auto& filterDrive = *apvts.getRawParameterValue(ParameterID::filterDrive);
    auto& filterKeyTrack = *apvts.getRawParameterValue(ParameterID::filterKeyTrack);
    auto& filterEnvAmt = *apvts.getRawParameterValue(ParameterID::filterEnvAmt);
    auto& filterType = *apvts.getRawParameterValue(ParameterID::filterType);
    
    auto& fenvAttack = *apvts.getRawParameterValue(ParameterID::fenvAttack);
    auto& fenvDecay = *apvts.getRawParameterValue(ParameterID::fenvDecay);
    auto& fenvSustain = *apvts.getRawParameterValue(ParameterID::fenvSustain);
    auto& fenvRelease = *apvts.getRawParameterValue(ParameterID::fenvRelease);
    auto& fenvCurve = *apvts.getRawParameterValue(ParameterID::fenvCurve);
    
    auto& aenvAttack = *apvts.getRawParameterValue(ParameterID::aenvAttack);
    auto& aenvDecay = *apvts.getRawParameterValue(ParameterID::aenvDecay);
    auto& aenvSustain = *apvts.getRawParameterValue(ParameterID::aenvSustain);
    auto& aenvRelease = *apvts.getRawParameterValue(ParameterID::aenvRelease);
    auto& aenvCurve = *apvts.getRawParameterValue(ParameterID::aenvCurve);
    
    auto& lfo1Rate = *apvts.getRawParameterValue(ParameterID::lfo1Rate);
    auto& lfo1Wave = *apvts.getRawParameterValue(ParameterID::lfo1Wave);
    auto& lfo1Sync = *apvts.getRawParameterValue(ParameterID::lfo1Sync);
    auto& lfo1FadeIn = *apvts.getRawParameterValue(ParameterID::lfo1FadeIn);
    
    auto& lfo2Rate = *apvts.getRawParameterValue(ParameterID::lfo2Rate);
    auto& lfo2Wave = *apvts.getRawParameterValue(ParameterID::lfo2Wave);
    auto& lfo2Sync = *apvts.getRawParameterValue(ParameterID::lfo2Sync);
    auto& lfo2FadeIn = *apvts.getRawParameterValue(ParameterID::lfo2FadeIn);
    
    oscillators[0].setWaveform(static_cast<Oscillator::Waveform>(osc1Wave));
    oscillators[0].setLevel(osc1Level);
    oscillators[0].setPulseWidth(osc1PW);
    
    oscillators[1].setWaveform(static_cast<Oscillator::Waveform>(osc2Wave));
    oscillators[1].setLevel(osc2Level);
    oscillators[1].setPulseWidth(osc2PW);
    
    oscillators[2].setWaveform(static_cast<Oscillator::Waveform>(osc3Wave));
    oscillators[2].setLevel(osc3Level);
    oscillators[2].setPulseWidth(osc3PW);
    
    filter.setType(static_cast<Filter::Type>(filterType));
    filter.setDrive(filterDrive);
    
    filterEnv.setAttack(fenvAttack);
    filterEnv.setDecay(fenvDecay);
    filterEnv.setSustain(fenvSustain);
    filterEnv.setRelease(fenvRelease);
    filterEnv.setCurve(fenvCurve);
    
    ampEnv.setAttack(aenvAttack);
    ampEnv.setDecay(aenvDecay);
    ampEnv.setSustain(aenvSustain);
    ampEnv.setRelease(aenvRelease);
    ampEnv.setCurve(aenvCurve);
    
    lfo1.setWaveform(static_cast<LFO::Waveform>(lfo1Wave));
    lfo1.setRate(lfo1Rate);
    lfo1.setSync(static_cast<LFO::SyncMode>(lfo1Sync));
    lfo1.setFadeIn(lfo1FadeIn);
    
    lfo2.setWaveform(static_cast<LFO::Waveform>(lfo2Wave));
    lfo2.setRate(lfo2Rate);
    lfo2.setSync(static_cast<LFO::SyncMode>(lfo2Sync));
    lfo2.setFadeIn(lfo2FadeIn);
    
    updateUnisonParameters();
}

void SynthVoice::updateUnisonParameters()
{
    unisonVoicesVec.resize(unisonVoices);
    for (int i = 0; i < unisonVoices; ++i) {
        float detune = (unisonVoices > 1) ? (float(i) / float(unisonVoices - 1) - 0.5f) * 2.0f * unisonDetune : 0.0f;
        unisonVoicesVec[i].detuneOffset = detune;
    }
}

bool SynthVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<juce::SynthesiserSound*>(sound) != nullptr;
}

void SynthVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition)
{
    currentPitchWheel = float(currentPitchWheelPosition) / 8192.0f;
    lastVelocity = velocity;
    
    float baseFreq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    
    float pitchBendRange = 2.0f;
    float pb = currentPitchWheel * pitchBendRange / 12.0f;
    baseFreq *= std::pow(2.0f, pb);
    
    for (int i = 0; i < 3; ++i) {
        oscillators[i].setFrequency(baseFreq);
    }
    subOsc.setFrequency(baseFreq * 0.5f);
    
    for (int i = 0; i < unisonVoices; ++i) {
        float detuneOffset = unisonVoicesVec[i].detuneOffset;
        float unisonFreq = baseFreq * std::pow(2.0f, detuneOffset / 1200.0f);
        
        for (int j = 0; j < 3; ++j) {
            unisonVoicesVec[i].oscillators[j].setFrequency(unisonFreq);
        }
    }
    
    filterEnv.noteOn();
    ampEnv.noteOn();
    lfo1.noteOn();
    lfo2.noteOn();
    
    for (auto& uv : unisonVoicesVec) {
        uv.filterEnv.noteOn();
        uv.ampEnv.noteOn();
    }
}

void SynthVoice::stopNote (float velocity, bool allowTailOff)
{
    if (allowTailOff) {
        filterEnv.noteOff();
        ampEnv.noteOff();
        
        for (auto& uv : unisonVoicesVec) {
            uv.filterEnv.noteOff();
            uv.ampEnv.noteOff();
        }
    } else {
        clearCurrentNote();
    }
}

void SynthVoice::pitchWheelMoved (int newPitchWheelValue)
{
    currentPitchWheel = float(newPitchWheelValue) / 8192.0f;
}

void SynthVoice::controllerMoved (int controllerNumber, int newControllerValue)
{
    float val = float(newControllerValue) / 127.0f;
    if (controllerNumber == 1) currentModWheel = val;
    else if (controllerNumber == 11) currentExpression = val;
    else if (controllerNumber == 64) sustainPedalDown = val > 0.5f;
}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (!isVoiceActive()) return;
    
    float lfo1Out = lfo1.process();
    float lfo2Out = lfo2.process();
    
    float fenvOut = filterEnv.process();
    float aenvOut = ampEnv.process();
    
    if (modMatrix) {
        modMatrix->updateSourceValues(lfo1, lfo2, filterEnv, ampEnv,
                                      lastVelocity, currentModWheel, currentAftertouch, currentPitchWheel);
    }
    
    float filterEnvAmt = 0.0f;
    if (modMatrix) filterEnvAmt = modMatrix->getModulatedValue(ModMatrix::FilterCutoff, fenvOut);
    
    for (int sample = 0; sample < numSamples; ++sample) {
        float lfo1Val = lfo1Out;
        float lfo2Val = lfo2Out;
        
        float oscMix = 0.0f;
        for (int i = 0; i < 3; ++i) {
            oscMix += oscillators[i].process();
        }
        
        float subVal = subOsc.process();
        float noiseVal = noiseGen.nextFloat();
        
        oscMix += subVal + noiseVal;
        
        float filtered = filter.process(oscMix);
        float output = filtered * aenvOut;
        
        buffer.addSample(0, startSample + sample, output);
        buffer.addSample(1, startSample + sample, output);
        
        if (sample < numSamples - 1) {
            lfo1Out = lfo1.process();
            lfo2Out = lfo2.process();
            fenvOut = filterEnv.process();
            aenvOut = ampEnv.process();
        }
    }
    
    if (!filterEnv.isActive() && !ampEnv.isActive()) {
        clearCurrentNote();
    }
}

float SynthVoice::getModulatedValue(int paramIndex, float baseValue)
{
    if (modMatrix) {
        return modMatrix->getModulatedValue(paramIndex, baseValue);
    }
    return baseValue;
}