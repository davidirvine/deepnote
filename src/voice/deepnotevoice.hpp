#pragma once

#include "daisysp.h"
#include "ranges/range.hpp"
#include "ranges/scaler.hpp"
#include "unitshapers/bezier.hpp"

namespace deepnote 
{

struct TraceValues
{
    TraceValues(const Range startRange, const float targetFreq, const int in_state, const int out_state, 
        const float animationLfo_value, const float shapedAnimationValue, const float animationFreq,
        const float frequency, const float oscValue) :  
    startRange(startRange),
    targetFreq(targetFreq),
    in_state(in_state),
    out_state(out_state),
    animationLfo_value(animationLfo_value),
    shapedAnimationValue(shapedAnimationValue),
    animationFreq(animationFreq),
    frequency(frequency),
    oscValue(oscValue)
    {}

    Range startRange;
    float targetFreq;
    int in_state;
    int out_state;
    float animationLfo_value;
    float shapedAnimationValue;
    float animationFreq;
    float frequency;
    float oscValue;
};


struct NoopTrace 
{
    inline void operator()(const TraceValues& values) const
    {}
};


template <size_t numOscillators = 1>
class DeepnoteVoice 
{
public:
    DeepnoteVoice() :
        startFrequencyRange({RangeLow(0.f), RangeHigh(0.f)}),
        startFrequency(0.f),
        targetFrequency(0.f),
        animationScaler(InputRange(Range(RangeLow(0.f), RangeHigh(1.f))), OutputRange(Range(RangeLow(0.f), RangeHigh(0.f)))),
        animationShaper(ControlPoint1(0.8f), ControlPoint2(0.5f))
    {}


    DeepnoteVoice(const Range startFreqencyRange, const float targetFrequency) :
        startFrequencyRange(startFreqencyRange),
        targetFrequency(targetFrequency),
        animationScaler(InputRange(Range(RangeLow(0.f), RangeHigh(1.f))), OutputRange(Range(RangeLow(0.f), RangeHigh(0.f)))),
        animationShaper(ControlPoint1(0.8f), ControlPoint2(0.5f))
    {}

    DeepnoteVoice(const DeepnoteVoice& other) :
        startFrequencyRange(other.startFrequencyRange),
        startFrequency(other.startFrequency),
        targetFrequency(other.targetFrequency),
        validFrequencyRange(other.validFrequencyRange),
        animationScaler(other.animationScaler),
        animationShaper(other.animationShaper)
    {}
    
    ~DeepnoteVoice() 
    {}

    template<typename F>
    void Init(const float sampleRate, const float animationRate, const F& random)
    {
        startFrequency = random(startFrequencyRange.GetLow(), startFrequencyRange.GetHigh());
        validFrequencyRange = Range(
            RangeLow(startFrequency < targetFrequency ? startFrequencyRange.GetLow() : startFrequencyRange.GetHigh()), 
            RangeHigh(targetFrequency));

        animationScaler = Scaler(
            InputRange(Range(RangeLow(0.f), RangeHigh(1.f))), 
            OutputRange(Range(RangeLow(startFrequency), RangeHigh(targetFrequency))));

        animationLfo.Init(sampleRate);
        animationLfo.SetAmp(0.5f);
        animationLfo.SetFreq(animationRate);  
        animationLfo.SetWaveform(daisysp::Oscillator::WAVE_RAMP);
        
        initOscillators(sampleRate, daisysp::Oscillator::WAVE_POLYBLEP_SAW, startFrequency, DEFAULT_DETUNE_INCREMENT);
    }
    
    template<typename F>
    float Process(const F& traceFunctor)
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
        auto shapedAnimationValue = animationShaper(animationLfoValue);

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
                animationFreq = animationScaler(shapedAnimationValue);
                frequency = animationFreq;
                break;
        }

        if (validFrequencyRange.Contains(frequency))
        {
            //  The frequency is within the valid range, so check to see if we've
            //  reached the start or target frequency
            if (state == IN_TRANSIT_TO_START)
            {
                const Range r{RangeLow(startFrequency - 1), RangeHigh(startFrequency + 1)};
                if  (r.Contains(frequency))
                {
                    state = AT_START;
                    frequency = startFrequency;
                }
            }
            else if (state == IN_TRANSIT_TO_TARGET)
            {
                const Range r{RangeLow(targetFrequency - 1), RangeHigh(targetFrequency + 1)};
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

        //  Give the traceFunctor a chance to log the state of the voice
        traceFunctor(TraceValues(startFrequencyRange, targetFrequency, in_state, state,
                        animationLfoValue, shapedAnimationValue, animationFreq, frequency, oscValue));

        return oscValue;
    }

    void SetAnimationRate(const float rate) 
    {
        animationLfo.SetFreq(rate);
    }

    void SetTargetFrequency(const float frequency) 
    {
        targetFrequency = frequency;
    }

    void computeDetune(const float detuneIncrement)
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

    void TransitionToTarget()
    {
        if (state != AT_TARGET && state != IN_TRANSIT_TO_TARGET)
        {
            state = PENDING_TRANSIT_TO_TARGET;
        }
    }

    void TransitionToStart()
    {
        if (state != AT_START && state != IN_TRANSIT_TO_START) 
        {
            state = PENDING_TRANSIT_TO_START;
        }
    }

    bool IsAtStart() const 
    {
        return state == AT_START;
    }

    bool IsAtTarget() const 
    {
        return state == AT_TARGET;
    }

private:

    void initOscillators(const float sampleRate, const uint8_t waveform, const float frequency, const float detuneIncrement)
    {
        for (auto& osc : oscillators) {
            osc.oscillator.Init(sampleRate);
            osc.oscillator.SetWaveform(waveform);
            osc.oscillator.SetFreq(frequency);
        }
        
        computeDetune(detuneIncrement);
    }
 
    float oscillatorsProcess(const float frequency)
    {
        float oscValue{0.f};
        for (auto& osc : oscillators) {
                osc.oscillator.SetFreq(frequency + osc.detuneAmount);
            oscValue += osc.oscillator.Process();
        }
        return oscValue;
    }

    const float DEFAULT_DETUNE_INCREMENT{2.5f};

    enum State 
    {
        AT_START = 0,
        PENDING_TRANSIT_TO_TARGET,
        IN_TRANSIT_TO_TARGET,
        AT_TARGET,
        PENDING_TRANSIT_TO_START,
        IN_TRANSIT_TO_START
    };

    struct DetunedOscillator 
    {
        daisysp::Oscillator oscillator;
        float detuneAmount;
    };

    State state{AT_START};
    Range startFrequencyRange{RangeLow(0.f), RangeHigh(1.f)};
    float startFrequency{0.f};
    float targetFrequency{0.f};
    Range validFrequencyRange{RangeLow(0.f), RangeHigh(1.f)};
    DetunedOscillator oscillators[numOscillators];
    daisysp::Oscillator animationLfo;
    Scaler animationScaler;
    BezierUnitShaper animationShaper;
};

} // namespace deepnote