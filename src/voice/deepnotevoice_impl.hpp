#pragma once

#include "deepnotevoice.hpp"
#include "scaler.hpp"

namespace deepnote
{
    
template <typename RandomT, size_t numOscillators, typename TraceT>
void DeepnoteVoice<RandomT, numOscillators, TraceT>::Init(const float sampleRate, const float animationRate) 
{
    startFrequency = random.GetFloat(startFrequencyRange.getLow(), startFrequencyRange.getHigh());
    validFrequencyRange = Range{startFrequency < targetFrequency ? startFrequencyRange.getLow() : startFrequencyRange.getHigh(), targetFrequency};

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

    const float ANIMATION_LFO_OFFSET{0.5f}; //  animationLfo is requrired to return values between -0.5 and 0.5
    const auto animationLfoValue = animationLfo.Process() + ANIMATION_LFO_OFFSET;
    auto shapedAnimationValue = animationShaper.shape(animationLfoValue);
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
            if ((startFrequency > targetFrequency) || (state == IN_TRANSIT_TO_START))
            {
                shapedAnimationValue = 1.f - shapedAnimationValue;
            }

            animationFreq = animationScaler.scale(shapedAnimationValue);
            frequency = animationFreq;
            break;
    }

    if (!validFrequencyRange.contains(frequency))
    {
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
        frequency = validFrequencyRange.constrain(frequency);
    }
    else
    {
        if (state == IN_TRANSIT_TO_START)
        {
            const Range r{startFrequency - 1, startFrequency + 1};
            if  (r.contains(frequency))
            {
                state = AT_START;
                frequency = startFrequency;
            }
        }
        else if (state == IN_TRANSIT_TO_TARGET)
        {
            const Range r{targetFrequency - 1, targetFrequency + 1};
            if (r.contains(frequency))
            {
                state = AT_TARGET;
                frequency = targetFrequency;
            }
        }
    }

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

} // namespace deepnote