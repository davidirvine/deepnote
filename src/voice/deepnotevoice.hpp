#pragma once

#include "voice/frequencytable.hpp"
#include "ranges/range.hpp"
#include "ranges/scaler.hpp"
#include "unitshapers/bezier.hpp"
#include "Synthesis/oscillator.h"

namespace deepnote 
{
namespace nt {
    using SampleRate = NamedType<float, struct SampleRateTag>;
    using AnimationMultiplier = NamedType<float, struct AnimationMultiplierTag>;
    using DetuneHz = NamedType<float, struct DetuneHzTag>;
}


struct TraceValues
{
    TraceValues(const float startFreq, const float targetFreq, const int in_state, const int out_state, 
        const float animationLfo_value, const float shapedAnimationValue, const float animationFreq,
        const float currentFrequency, const float oscValue) :  
    startFreq(startFreq),
    targetFreq(targetFreq),
    in_state(in_state),
    out_state(out_state),
    animationLfo_value(animationLfo_value),
    shapedAnimationValue(shapedAnimationValue),
    animationFreq(animationFreq),
    currentFrequency(currentFrequency),
    oscValue(oscValue)
    {}

    float startFreq;
    float targetFreq;
    int in_state;
    int out_state;
    float animationLfo_value;
    float shapedAnimationValue;
    float animationFreq;
    float currentFrequency;
    float oscValue;
};


struct NoopTrace 
{
    inline void operator()(const TraceValues& values) const
    {}
};


constexpr float ANIMATION_LFO_AMPLITUDE{0.5f};

template <int numOscillators = 1>
class DeepnoteVoice 
{
public:

    DeepnoteVoice()
    {}

    
    virtual ~DeepnoteVoice() 
    {}

    template<typename F>
    void Init(const nt::OscillatorFrequency& startFrequency, const nt::SampleRate& sampleRate, 
                const nt::OscillatorFrequency& animationFrequency, const F& random)
    {
        this->startFrequency = startFrequency;
        this->targetFrequency = startFrequency;
        this->currentFrequency = startFrequency;
        this->baseAnimationFrequency = animationFrequency;
        animationLfo.Init(sampleRate.get());
        animationLfo.SetAmp(ANIMATION_LFO_AMPLITUDE);
        animationLfo.SetFreq(baseAnimationFrequency.get());  
        animationLfo.SetWaveform(daisysp::Oscillator::WAVE_RAMP);
        
        initOscillators(sampleRate, daisysp::Oscillator::WAVE_POLYBLEP_SAW, currentFrequency, DEFAULT_DETUNE_INCREMENT);
    }
    
    template<typename F>
    float Process(const nt::AnimationMultiplier& animationMultiplier, 
        const nt::ControlPoint1& cp1, const nt::ControlPoint2& cp2, const F& traceFunctor)
    {
        const auto in_state{state};
        //  if we in a pending state, reset the animation LFO and move to the next state
        switch (state)
        {
            case PENDING_TRANSIT_TO_TARGET:
                state = IN_TRANSIT_TO_TARGET;
                animationLfo.Reset();
                break;

            default:
                break;
        }

        //  Take the linear ramp of the animation LFO and apply the shaper to it
        //  This will give us some non-linear frequency changes
        //  The animation LFO has an range of -0.5 to 0.5, so we need to offset it
        //  The shaper takes and returns values from 0.0 to 1.0
        animationLfo.SetFreq(animationMultiplier.get() * baseAnimationFrequency.get());
        const BezierUnitShaper animationShaper(cp1, cp2);
        const auto animationLfoValue = animationLfo.Process() + ANIMATION_LFO_AMPLITUDE;
        auto shapedAnimationValue = animationShaper(animationLfoValue);

        nt::OscillatorFrequency animationFreq(0.f);

        switch (state) 
        {
            case AT_TARGET:
                currentFrequency = targetFrequency;
                break;

            default:
                //  If we want the frequency to decrease we need to flip the shaped value
                if (startFrequency.get() > targetFrequency.get())
                {
                    shapedAnimationValue = 1.f - shapedAnimationValue;
                } 

                //  Scale the 0.0 to 1.0 shaped value to the start and target frequency range
                const auto animationScaler = Scaler(
                    nt::InputRange(Range(
                        nt::RangeLow(0.f), 
                        nt::RangeHigh(1.f))
                    ), 
                    nt::OutputRange(Range(
                        nt::RangeLow(startFrequency.get()), 
                        nt::RangeHigh(targetFrequency.get())
                    ))
                );

                animationFreq = nt::OscillatorFrequency(animationScaler(shapedAnimationValue));
                currentFrequency = animationFreq;
                break;
        }

        const nt::OscillatorFrequencyRange validFrequencyRange(
            Range(
                nt::RangeLow(startFrequency.get()), 
                nt::RangeHigh(targetFrequency.get())
            )
        );

        if (validFrequencyRange.get().Contains(currentFrequency.get()))
        {
            //  The frequency is within the valid range, so check to see if we've
            //  reached the start or target frequency
            if (state == IN_TRANSIT_TO_TARGET)
            {
                const nt::OscillatorFrequencyRange targetRange(
                    Range(
                        nt::RangeLow(targetFrequency.get() - 1), 
                        nt::RangeHigh(targetFrequency.get() + 1)
                    )
                );

                if (targetRange.get().Contains(currentFrequency.get()))
                {
                    state = AT_TARGET;
                    currentFrequency = targetFrequency;
                }
            }
        }
        else
        {
            // The frequency is outside the valid range, so constrain it and set the state
            switch (state)
            {
                case IN_TRANSIT_TO_TARGET:
                    state = AT_TARGET;
                    break;
                default:
                    break;
            }
            currentFrequency = nt::OscillatorFrequency(validFrequencyRange.get().Constrain(currentFrequency.get()));
        }
        
        //  Update all oscillators using the new frequency
        const auto oscValue = oscillatorsProcess();

        //  Give the traceFunctor a chance to log the state of the voice
        traceFunctor(TraceValues(
            startFrequency.get(), 
            targetFrequency.get(), 
            in_state, 
            state,
            animationLfoValue, 
            shapedAnimationValue, 
            animationFreq.get(), 
            currentFrequency.get(), 
            oscValue
        ));

        return oscValue;
    }

    void SetAnimationRate(const nt::OscillatorFrequency& rate) 
    {
        animationLfo.SetFreq(rate.get());
    }

    void SetTargetFrequency(const nt::OscillatorFrequency& targetFrequency) 
    {
        //  set up a new transit from something close to the current frequency of
        //  the voice and the new target frequency
        this->startFrequency = this->currentFrequency;
        this->targetFrequency = targetFrequency;
        this->state = PENDING_TRANSIT_TO_TARGET;
    }

    void computeDetune(const nt::DetuneHz& detune)
    {
        const auto half = numOscillators / 2;
        for (size_t i = 0; i < numOscillators; ++i) {
            auto& osc = oscillators[i];
            if (numOscillators <= 1) {
                osc.detuneAmount = 0.f;
            } else {
                const int8_t idx = i - half + ((i >= half) ? 1 : 0);
                osc.detuneAmount = idx * detune.get();   
            }
        }
    }

    bool IsAtTarget() const 
    {
        return state == AT_TARGET;
    }

private:

    void initOscillators(const nt::SampleRate& sampleRate, const uint8_t waveform, 
        const nt::OscillatorFrequency& frequency, const nt::DetuneHz& detune)
    {
        for (auto& osc : oscillators) {
            osc.oscillator.Init(sampleRate.get());
            osc.oscillator.SetWaveform(waveform);
            osc.oscillator.SetFreq(frequency.get());
        }
        
        computeDetune(detune);
    }
 
    float oscillatorsProcess()
    {
        float oscValue{0.f};
        for (auto& osc : oscillators) {
                osc.oscillator.SetFreq(currentFrequency.get() + osc.detuneAmount);
            oscValue += osc.oscillator.Process();
        }
        return oscValue;
    }

    const nt::DetuneHz DEFAULT_DETUNE_INCREMENT{2.5f};

    enum State 
    {
        PENDING_TRANSIT_TO_TARGET,
        IN_TRANSIT_TO_TARGET,
        AT_TARGET
    };

    struct DetunedOscillator 
    {
        daisysp::Oscillator oscillator;
        float detuneAmount;
    };

    State state{PENDING_TRANSIT_TO_TARGET};
    nt::OscillatorFrequency startFrequency{0.f};
    nt::OscillatorFrequency targetFrequency{0.f};
    nt::OscillatorFrequency currentFrequency{0.f};
    DetunedOscillator oscillators[numOscillators];
    nt::OscillatorFrequency baseAnimationFrequency{0.f};
    daisysp::Oscillator animationLfo;
    Scaler animationScaler;
};

} // namespace deepnote