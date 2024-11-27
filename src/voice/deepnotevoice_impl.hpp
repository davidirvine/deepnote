#pragma once

#include "deepnotevoice.hpp"
#include "scaler.hpp"


template <typename RandomT, size_t numOscillators, typename TraceT>
void DeepnoteVoice<RandomT, numOscillators, TraceT>::Init(const float sampleRate, const float animationRate) 
{
    initOscillators(sampleRate, daisysp::Oscillator::WAVE_POLYBLEP_SAW, startFrequency, DEFAULT_DETUNE_INCREMENT);

    //  
    //  animationLfo will be used to move the oscillator frequency from start to target
    //
    animationLfo.Init(sampleRate);
    animationLfo.SetAmp(0.5f);  // range of values should be between -1 and 1
    animationLfo.SetFreq(animationRate);    // the animation will complete in 1 full cycle
    animationLfo.SetWaveform(daisysp::Oscillator::WAVE_RAMP);
}

template <typename RandomT, size_t numOscillators, typename TraceT>
void DeepnoteVoice<RandomT, numOscillators, TraceT>::initOscillators(const float sampleRate, const uint8_t waveform, 
    const float frequency, const float detuneIncrement) 
{
    for (auto& osc : oscillators) {
        osc.oscillator.Init(sampleRate);
        osc.oscillator.SetWaveform(waveform);
        osc.oscillator.SetFreq(frequency);
    }

    computeDetune(detuneIncrement);
}


template <typename RandomT, size_t numOscillators, typename TraceT>
void DeepnoteVoice<RandomT, numOscillators, TraceT>::computeDetune(const float detuneIncrement) 
{
    const size_t half = numOscillators / 2;
    for (size_t i = 0; i < numOscillators; ++i) {
        auto& osc = oscillators[i];
        if (numOscillators <= 1) {
            osc.detuneAmount = 0.f;
        } else {
            const int8_t idx = i - half + ((i >= half) ? 1 : 0);
            osc.detuneAmount = idx * detuneIncrement;   
        }
    }
}


template <typename RandomT, size_t numOscillators, typename TraceT>
float DeepnoteVoice<RandomT, numOscillators, TraceT>::oscillatorsProcess(const float frequency) 
{
    float oscValue{0.f};
    for (auto& osc : oscillators) {
            osc.oscillator.SetFreq(frequency + osc.detuneAmount);
        oscValue += osc.oscillator.Process();
    }
    return oscValue;
}


template <typename RandomT, size_t numOscillators, typename TraceT>
float DeepnoteVoice<RandomT, numOscillators, TraceT>::Process() 
{
    const auto ANIMATION_LFO_OFFSET{0.5f}; //  animationLfo is requrired to return values between -0.5 and 0.5
    const auto in_state{state};
    const auto animationLfoValue = animationLfo.Process() + ANIMATION_LFO_OFFSET;
    auto shapedAnimationValue = animationShaper.shape(animationLfoValue);

    //
    //  if we're heading back to the start invert the shapedAnimationValue
    //
    if (state == IN_TRANSIT_TO_START) {
        shapedAnimationValue = 1.f - shapedAnimationValue;
    }

    auto animationFreq{0.f};
    auto frequency{0.f};
    switch (state) {
        case AT_START:
            frequency = startFrequency;
            break;

        case AT_TARGET:
            frequency = targetFrequency;
            break;
        
        default:
            animationFreq = animationScaler.scale(shapedAnimationValue);
            frequency = animationFreq;
            break;
    }

    //
    //  frequency should be bounded by the startFrequencyRange and targetFrequency
    //
    if (frequency < startFrequencyRange.getLow()) {
        frequency = startFrequencyRange.getLow();
    } else if (frequency > targetFrequency) {
        frequency = targetFrequency;
    }

    //
    //  Update the state based on the current frequency
    //
    state = state == IN_TRANSIT_TO_START && startFrequencyRange.contains(frequency) ? AT_START : state;
    state = state == IN_TRANSIT_TO_TARGET && frequency == targetFrequency ? AT_TARGET : state;

    //
    //  Let the ocillators run
    //
    const auto oscValue = oscillatorsProcess(frequency);

    if (tracer != nullptr) {
        tracer->trace(startFrequencyRange, targetFrequency, in_state, state,
                    animationLfoValue, shapedAnimationValue, animationFreq, frequency, oscValue);
    }

    return oscValue;
}