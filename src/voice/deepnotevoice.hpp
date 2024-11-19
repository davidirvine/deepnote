#ifndef DEEPNOTEVOICE_HPP
#define DEEPNOTEVOICE_HPP

#include "daisysp.h"
#include "range.hpp"
#include "scaler.hpp"

using namespace daisysp;

namespace deepnotedrone {
    //
    //  DeepnoteVoice
    //
    class DeepnoteVoice {
    public:
        DeepnoteVoice() :
            startFrequencyRange({0.0, 0.0}),
            frequency(0.0),
            targetFrequency(0.0),
            detuneWeight(0.0),
            animationScaler({0.0, 1.0}, {0.0, 0.0})
        {}

        DeepnoteVoice(Range startFreqencyRange, float targetFrequency, float detuneWeight) :
            startFrequencyRange(startFreqencyRange),
            frequency(startFrequencyRange.random()),
            targetFrequency(targetFrequency),
            detuneWeight(detuneWeight),
            animationScaler({0.0, 1.0}, {startFrequencyRange.getLow(), targetFrequency})
        {}

        ~DeepnoteVoice() 
        {}

        void Init(float sampleRate, float detuneRate, float animationRate);
        
        float Process();

        void SetAnimationRate(float rate) {
            animationLfo.SetFreq(rate);
        }

        void SetTargetFrequency(float frequency) {
            targetFrequency = frequency;
        }


    private:

        enum State {
            AT_START,
            IN_TRANSIT_TO_TARGET,
            AT_TARGET,
            IN_TRANSIT_TO_START
        };

        State state{ AT_START };
        Range startFrequencyRange{ 0.0, 0.0 };
        float frequency{0.0};
        float targetFrequency{0.0};
        float detuneWeight{0.0};
        Oscillator oscillator;
        Oscillator detuneLfo;
        Oscillator animationLfo;
        Scaler animationScaler;
    };
}

#endif // DEEPNOTEVOICE_HPP