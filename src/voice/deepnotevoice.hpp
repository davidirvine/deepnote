#ifndef DEEPNOTEVOICE_HPP
#define DEEPNOTEVOICE_HPP

#include "daisysp.h"
#include "range.hpp"
#include "scaler.hpp"
#include "bezier.hpp"

using namespace daisysp;

namespace deepnotedrone {

    //
    //  NullTraceType
    //
    struct NullTraceType {
        void trace(const Range startRange, const float targetFreq, const int in_state, const int out_state, 
            const float detuneLfo_value, const float animationLfo_value, const float shapedAnimationValue,
            const float detune, const float animationFreq, const float frequency, const float oscValue) 
        {}
    };

    //
    //  DeepnoteVoice
    //
    template <typename RandomT, typename TraceT = NullTraceType>
    class DeepnoteVoice {
    public:
        DeepnoteVoice() :
            startFrequencyRange({0.0f, 0.0f}),
            startFrequency(0.0f),
            targetFrequency(0.0f),
            detuneWeight(0.0f),
            animationScaler({0.0f, 1.0f}, {0.0f, 0.0f}),
            animationShaper(0.8f, 0.5f)
        {}


        DeepnoteVoice(const Range startFreqencyRange, const float targetFrequency, const float detuneWeight) :
            startFrequencyRange(startFreqencyRange),
            startFrequency(RandomT::GetRandomFloat(startFrequencyRange.getLow(), startFrequencyRange.getHigh())),
            targetFrequency(targetFrequency),
            detuneWeight(detuneWeight),
            animationScaler({0.0f, 1.0f}, {startFrequency, targetFrequency}),
            animationShaper(0.8f, 0.5f)
        {}

        ~DeepnoteVoice() 
        {}

        void Init(const float sampleRate, const float detuneRate, const float animationRate);
        
        float Process();

        void SetAnimationRate(const float rate) {
            animationLfo.SetFreq(rate);
        }

        void SetTargetFrequency(const float frequency) {
            targetFrequency = frequency;
        }

        void SetTracer(TraceT* tracer) {
            this->tracer = tracer;
        }

    private:

        enum State {
            AT_START = 0,
            IN_TRANSIT_TO_TARGET,
            AT_TARGET,
            IN_TRANSIT_TO_START
        };

        State state{IN_TRANSIT_TO_TARGET};
        Range startFrequencyRange{0.0f, 0.0f};
        float startFrequency{0.0f};
        float targetFrequency{0.0f};
        float detuneWeight{0.0f};
        Oscillator oscillator;
        Oscillator detuneLfo;
        Oscillator animationLfo;
        Scaler animationScaler;
        BezierUnitShaper animationShaper;
        TraceT* tracer{nullptr};
    };
}

#include "deepnotevoice_impl.hpp"

#endif // DEEPNOTEVOICE_HPP