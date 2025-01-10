#pragma once

#include "daisysp.h"
#include "ranges/range.hpp"
#include "ranges/scaler.hpp"
#include "unitshapers/bezier.hpp"

namespace deepnote 
{

//
//  NullTraceType
//
struct NullTraceType 
{
    void trace(const Range startRange, const float targetFreq, const int in_state, const int out_state, 
        const float animationLfo_value, const float shapedAnimationValue, const float animationFreq, 
        const float frequency, const float oscValue) 
    {}
};

//
//  DeepnoteVoice
//
template <typename RandomT, size_t numOscillators, typename TraceT = NullTraceType>
class DeepnoteVoice 
{
public:
    DeepnoteVoice() :
        startFrequencyRange({0.f, 0.f}),
        startFrequency(0.f),
        targetFrequency(0.f),
        animationScaler({0.f, 1.f}, {0.f, 0.f}),
        animationShaper(0.8f, 0.5f)
    {}


    DeepnoteVoice(const Range startFreqencyRange, const float targetFrequency) :
        startFrequencyRange(startFreqencyRange),
        targetFrequency(targetFrequency),
        animationScaler({0.f, 1.f}, {0.f, 0.f}),
        animationShaper(0.8f, 0.5f)
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

    void Init(const float sampleRate, const float animationRate);
    
    float Process();

    void SetAnimationRate(const float rate) 
    {
        animationLfo.SetFreq(rate);
    }

    void SetTargetFrequency(const float frequency) 
    {
        targetFrequency = frequency;
    }

    void SetTracer(TraceT* tracer) 
    {
        this->tracer = tracer;
    }

    void computeDetune(const float detuneIncrement);

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

    void initOscillators(const float sampleRate, const uint8_t waveform, const float frequency, const float detuneIncrement);
    float oscillatorsProcess(const float frequency);

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
    Range startFrequencyRange{0.f, 0.f};
    float startFrequency{0.f};
    float targetFrequency{0.f};
    Range validFrequencyRange{0.f, 0.f};
    DetunedOscillator oscillators[numOscillators];
    daisysp::Oscillator animationLfo;
    Scaler animationScaler;
    BezierUnitShaper animationShaper;
    TraceT* tracer{nullptr};
    RandomT random;
};

} // namespace deepnote

#include "deepnotevoice_impl.hpp"
