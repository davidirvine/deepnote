#ifndef __DEEPNOTEVOICE_IMPL_HPP__
#define __DEEPNOTEVOICE_IMPL_HPP__

#include "deepnotevoice.hpp"
#include "scaler.hpp"

namespace deepnotedrone {

    template <typename RandomT, typename TraceT>
    void DeepnoteVoice<RandomT, TraceT>::Init(const float sampleRate, const float detuneRate, const float animationRate) {
        oscillator.Init(sampleRate);
        oscillator.SetFreq(startFrequency);
        oscillator.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);

        //
        //  detuneLfo will be used to modulate the frequency of the oscillator
        //  
        detuneLfo.Init(sampleRate);
        detuneLfo.SetAmp(0.5f);  // range of values should be between -1 and 1
        detuneLfo.SetFreq(detuneRate);
        detuneLfo.SetWaveform(Oscillator::WAVE_TRI);

        //  
        //  animationLfo will be used to move the oscillator frequency from start to target
        //
        animationLfo.Init(sampleRate);
        animationLfo.SetAmp(0.5f);  // range of values should be between -1 and 1
        animationLfo.SetFreq(animationRate);    // the animation will complete in 1 full cycle
        animationLfo.SetWaveform(Oscillator::WAVE_RAMP);
    }

    template <typename RandomT, typename TraceT>
    float DeepnoteVoice<RandomT, TraceT>::Process() {
        const auto ANIMATION_LFO_OFFSET{0.5f}; //  animationLfo is requrired to return values between -0.5 and 0.5
        const auto in_state{state};

        auto detuneLfoValue = detuneLfo.Process();
        auto detune = detuneWeight * detuneLfoValue;
        auto animationLfoValue = animationLfo.Process() + ANIMATION_LFO_OFFSET;
        auto shapedAnimationValue = animationShaper.shape(animationLfoValue);

        //
        //  if we're heading back to the start invert the shapedAnimationValue
        //
        if (state == IN_TRANSIT_TO_START) {
            shapedAnimationValue = 1.0f - shapedAnimationValue;
        }

        auto animationFreq{0.0f};
        auto frequency{0.0f};
        switch (state) {
            case AT_START:
                frequency = startFrequency + detune;
                break;

            case AT_TARGET:
                frequency = targetFrequency + detune;
                break;
            
            default:
                animationFreq = animationScaler.scale(shapedAnimationValue);
                frequency = animationFreq + detune;
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
        
        state = state == IN_TRANSIT_TO_START && startFrequencyRange.contains(frequency) ? AT_START : state;
        state = state == IN_TRANSIT_TO_TARGET && frequency == targetFrequency ? AT_TARGET : state;

        oscillator.SetFreq(frequency);
        const auto oscValue = oscillator.Process();

        if (tracer != nullptr) {
            tracer->trace(startFrequencyRange, targetFrequency, in_state, state, detuneLfoValue,
                             animationLfoValue, shapedAnimationValue, detune, animationFreq, frequency, oscValue);
        }

        return oscValue;
    }
}

#endif // __DEEPNOTEVOICE_IMPL_HPP__