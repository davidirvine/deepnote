#include "deepnotevoice.hpp"
#include "scaler.hpp"

namespace deepnotedrone {

     void DeepnoteVoice::Init(float sampleRate, float detuneRate, float animationRate) {
        oscillator.Init(sampleRate);
        oscillator.SetFreq(frequency);
        oscillator.SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);

        //
        //  detuneLfo will be used to modulate the frequency of the oscillator
        //  
        detuneLfo.Init(sampleRate);
        detuneLfo.SetAmp(0.5);  // range of values should be between -1 and 1
        detuneLfo.SetFreq(detuneRate);
        detuneLfo.SetWaveform(Oscillator::WAVE_TRI);

        //  
        //  animationLfo will be used to move the oscillator frequency from start to target
        //
        animationLfo.Init(sampleRate);
        animationLfo.SetAmp(0.5);  // range of values should be between -1 and 1
        animationLfo.SetFreq(animationRate);    // the animation will complete in 1 full cycle
        animationLfo.SetWaveform(Oscillator::WAVE_RAMP);
    }

    float DeepnoteVoice::Process() {
        const float DETUNE_LFO_OFFSET = 0.5;    //  detuneLfo is requrired to return values between -0.5 and 0.5
        const float ANIMATION_LFO_OFFSET = 0.5; //  animationLfo is requrired to return values between -0.5 and 0.5

        auto detuneLfo_value = detuneLfo.Process() + DETUNE_LFO_OFFSET;
        auto detune = detuneWeight * detuneLfo_value;
        auto animationLfo_value = animationLfo.Process();

        if (state == IN_TRANSIT_TO_START) {
            animationLfo_value = ANIMATION_LFO_OFFSET - animationLfo_value;
        }

        if (state == AT_START || state == AT_TARGET) {
            frequency = frequency + detune;
        } else {
            frequency = animationScaler.scale(animationLfo_value) + detune;
        }

        //
        //  frequency should be bounded by the startFrequencyRange and targetFrequency
        //
        if (frequency < startFrequencyRange.getLow()) {
            frequency = startFrequencyRange.getLow();
        } else if (frequency > targetFrequency) {
            frequency = targetFrequency;
        }
        
        state = startFrequencyRange.contains(frequency) ? AT_START : state;
        state = frequency == targetFrequency ? AT_TARGET : state;
        
        oscillator.SetFreq(frequency);
        return oscillator.Process();
    }
}
