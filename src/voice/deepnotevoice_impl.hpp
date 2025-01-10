#pragma once

#include "deepnotevoice.hpp"
#include "ranges/scaler.hpp"

namespace deepnote
{
    
template <typename RandomT, size_t numOscillators, typename TraceT>
void DeepnoteVoice<RandomT, numOscillators, TraceT>::Init(const float sampleRate, const float animationRate) 
{
    startFrequency = random.GetFloat(startFrequencyRange.GetLow(), startFrequencyRange.GetHigh());
    validFrequencyRange = Range{startFrequency < targetFrequency ? startFrequencyRange.GetLow() : startFrequencyRange.GetHigh(), targetFrequency};

    animationScaler = Scaler(Range{0.f, 1.f}, Range{startFrequency, targetFrequency});

    animationLfo.Init(sampleRate);
    animationLfo.SetAmp(0.5f);
    animationLfo.SetFreq(animationRate);  
    animationLfo.SetWaveform(daisysp::Oscillator::WAVE_RAMP);
    
    initOscillators(sampleRate, daisysp::Oscillator::WAVE_POLYBLEP_SAW, startFrequency, DEFAULT_DETUNE_INCREMENT);
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
    const auto half = numOscillators / 2;
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
    const auto in_state{state};
    //  if we in a pending state, reset the animation LFO and move to the next state
    switch (state)
    {
        case PENDING_TRANSIT_TO_TARGET:
            state = IN_TRANSIT_TO_TARGET;
            animationLfo.Reset();
            break;

        case PENDING_TRANSIT_TO_START:
            state = IN_TRANSIT_TO_START;
            animationLfo.Reset();
            break;
        default:
            break;
    }

    //  Take the linear ramp of the animation LFO and apply the shaper to it
    //  This will give us some non-linear frequency changes
    //  The animation LFO has an range of -0.5 to 0.5, so we need to offset it
    //  The shaper takes and returns values from 0.0 to 1.0
    const float ANIMATION_LFO_OFFSET{0.5f};
    const auto animationLfoValue = animationLfo.Process() + ANIMATION_LFO_OFFSET;
    auto shapedAnimationValue = animationShaper.Shape(animationLfoValue);

    float animationFreq{0.f};
    float frequency{0.f};

    switch (state) 
    {
        case AT_START:
            frequency = startFrequency;
            break;

        case AT_TARGET:
            frequency = targetFrequency;
            break;

        default:
            //  If we want the frequency to decrease we need to flip the shaped value
            if ((startFrequency > targetFrequency) || (state == IN_TRANSIT_TO_START))
            {
                shapedAnimationValue = 1.f - shapedAnimationValue;
            }

            //  Scale the 0.0 to 1.0 shaped value to the start and target frequency range
            animationFreq = animationScaler.Scale(shapedAnimationValue);
            frequency = animationFreq;
            break;
    }

    if (validFrequencyRange.Contains(frequency))
    {
        //  The frequency is within the valid range, so check to see if we've
        //  reached the start or target frequency
        if (state == IN_TRANSIT_TO_START)
        {
            const Range r{startFrequency - 1, startFrequency + 1};
            if  (r.Contains(frequency))
            {
                state = AT_START;
                frequency = startFrequency;
            }
        }
        else if (state == IN_TRANSIT_TO_TARGET)
        {
            const Range r{targetFrequency - 1, targetFrequency + 1};
            if (r.Contains(frequency))
            {
                state = AT_TARGET;
                frequency = targetFrequency;
            }
        }
    }
    else
    {
        // The frequency is outside the valid range, so clamp it and set the state
        switch (state)
        {
            case IN_TRANSIT_TO_START:
                state = AT_START;
                break;
            case IN_TRANSIT_TO_TARGET:
                state = AT_TARGET;
                break;
            default:
                break;
        }
        frequency = validFrequencyRange.Constrain(frequency);
    }
    
    //  Update all oscillators using the new frequency
    const auto oscValue = oscillatorsProcess(frequency);

    //  If we have a tracer, trace the current state
    if (tracer != nullptr) {
        tracer->trace(startFrequencyRange, targetFrequency, in_state, state,
                    animationLfoValue, shapedAnimationValue, animationFreq, frequency, oscValue);
    }

    return oscValue;
}

} // namespace deepnote