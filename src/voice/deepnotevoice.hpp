#pragma once

#include "voice/frequencytable.hpp"
#include "ranges/range.hpp"
#include "ranges/scaler.hpp"
#include "unitshapers/bezier.hpp"
#include "Synthesis/oscillator.h"

namespace deepnote
{
    namespace nt
    {
        using SampleRate = NamedType<float, struct SampleRateTag>;
        using AnimationMultiplier = NamedType<float, struct AnimationMultiplierTag>;
        using DetuneHz = NamedType<float, struct DetuneHzTag>;
    }

    struct TraceValues
    {
        TraceValues(const float start_freq, const float target_freq, const int in_state, const int out_state,
                    const float animation_lfo_value, const float shaped_animation_value, const float animation_freq,
                    const float current_frequency, const float osc_value) : start_freq(start_freq),
                                                                            target_freq(target_freq),
                                                                            in_state(in_state),
                                                                            out_state(out_state),
                                                                            animation_lfo_value(animation_lfo_value),
                                                                            shaped_animation_value(shaped_animation_value),
                                                                            animation_freq(animation_freq),
                                                                            current_frequency(current_frequency),
                                                                            osc_value(osc_value)
        {
        }

        float start_freq;
        float target_freq;
        int in_state;
        int out_state;
        float animation_lfo_value;
        float shaped_animation_value;
        float animation_freq;
        float current_frequency;
        float osc_value;
    };

    struct NoopTrace
    {
        inline void operator()(const TraceValues &values) const
        {
        }
    };

    constexpr float ANIMATION_LFO_AMPLITUDE{0.5f};

    template <int num_oscillators = 1>
    class DeepnoteVoice
    {
    public:
        DeepnoteVoice()
        {
        }

        virtual ~DeepnoteVoice()
        {
        }

        template <typename F>
        void init(const nt::OscillatorFrequency &start_frequency, const nt::SampleRate &sample_rate,
                  const nt::OscillatorFrequency &animation_frequency, const F &random)
        {
            this->start_frequency = start_frequency;
            this->target_frequency = start_frequency;
            this->current_frequency = start_frequency;
            this->base_animation_frequency = animation_frequency;
            animation_lfo.Init(sample_rate.get());
            animation_lfo.SetAmp(ANIMATION_LFO_AMPLITUDE);
            animation_lfo.SetFreq(base_animation_frequency.get());
            animation_lfo.SetWaveform(daisysp::Oscillator::WAVE_RAMP);

            init_oscillators(sample_rate, daisysp::Oscillator::WAVE_POLYBLEP_SAW, current_frequency, DEFAULT_DETUNE_INCREMENT);
        }

        template <typename F>
        float process(const nt::AnimationMultiplier &animation_multiplier,
                      const nt::ControlPoint1 &cp1, const nt::ControlPoint2 &cp2, const F &trace_functor)
        {
            const auto in_state{state};
            //  if we in a pending state, reset the animation LFO and move to the next state
            switch (state)
            {
            case PENDING_TRANSIT_TO_TARGET:
                state = IN_TRANSIT_TO_TARGET;
                animation_lfo.Reset();
                break;

            default:
                break;
            }

            //  Take the linear ramp of the animation LFO and apply the shaper to it
            //  This will give us some non-linear frequency changes
            //  The animation LFO has an range of -0.5 to 0.5, so we need to offset it
            //  The shaper takes and returns values from 0.0 to 1.0
            animation_lfo.SetFreq(animation_multiplier.get() * base_animation_frequency.get());
            const BezierUnitShaper animation_shaper(cp1, cp2);
            const auto animation_lfo_value = animation_lfo.Process() + ANIMATION_LFO_AMPLITUDE;
            auto shaped_animation_value = animation_shaper(animation_lfo_value);

            nt::OscillatorFrequency animation_freq(0.f);

            switch (state)
            {
            case AT_TARGET:
                current_frequency = target_frequency;
                break;

            default:
                //  If we want the frequency to decrease we need to flip the shaped value
                if (start_frequency.get() > target_frequency.get())
                {
                    shaped_animation_value = 1.f - shaped_animation_value;
                }

                //  Scale the 0.0 to 1.0 shaped value to the start and target frequency range
                const auto animationScaler = Scaler(
                    nt::InputRange(Range(
                        nt::RangeLow(0.f),
                        nt::RangeHigh(1.f))),
                    nt::OutputRange(Range(
                        nt::RangeLow(start_frequency.get()),
                        nt::RangeHigh(target_frequency.get()))));

                animation_freq = nt::OscillatorFrequency(animationScaler(shaped_animation_value));
                current_frequency = animation_freq;
                break;
            }

            const nt::OscillatorFrequencyRange validFrequencyRange(
                Range(
                    nt::RangeLow(start_frequency.get()),
                    nt::RangeHigh(target_frequency.get())));

            if (validFrequencyRange.get().contains(current_frequency.get()))
            {
                //  The frequency is within the valid range, so check to see if we've
                //  reached the start or target frequency
                if (state == IN_TRANSIT_TO_TARGET)
                {
                    const nt::OscillatorFrequencyRange targetRange(
                        Range(
                            nt::RangeLow(target_frequency.get() - 1),
                            nt::RangeHigh(target_frequency.get() + 1)));

                    if (targetRange.get().contains(current_frequency.get()))
                    {
                        state = AT_TARGET;
                        current_frequency = target_frequency;
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
                current_frequency = nt::OscillatorFrequency(validFrequencyRange.get().constrain(current_frequency.get()));
            }

            //  Update all oscillators using the new frequency
            const auto osc_value = process_oscillators();

            //  Give the traceFunctor a chance to log the state of the voice
            trace_functor(TraceValues(
                start_frequency.get(),
                target_frequency.get(),
                in_state,
                state,
                animation_lfo_value,
                shaped_animation_value,
                animation_freq.get(),
                current_frequency.get(),
                osc_value));

            return osc_value;
        }

        void set_animation_rate(const nt::OscillatorFrequency &rate)
        {
            animation_lfo.SetFreq(rate.get());
        }

        void set_target_frequency(const nt::OscillatorFrequency &target_frequency)
        {
            //  set up a new transit from something close to the current frequency of
            //  the voice and the new target frequency
            this->start_frequency = this->current_frequency;
            this->target_frequency = target_frequency;
            this->state = PENDING_TRANSIT_TO_TARGET;
        }

        void reset_start_frequency(const nt::OscillatorFrequency &start_frequency)
        {
            //  set up a new transit from a new start frequency
            this->start_frequency = start_frequency;
            this->current_frequency = this->start_frequency;
            this->state = PENDING_TRANSIT_TO_TARGET;
        }

        void compute_detune(const nt::DetuneHz &detune)
        {
            const auto half = num_oscillators / 2;
            for (size_t i = 0; i < num_oscillators; ++i)
            {
                auto &osc = oscillators[i];
                if (num_oscillators <= 1)
                {
                    osc.detune_amount = 0.f;
                }
                else
                {
                    const int8_t idx = i - half + ((i >= half) ? 1 : 0);
                    osc.detune_amount = idx * detune.get();
                }
            }
        }

        bool is_at_target() const
        {
            return state == AT_TARGET;
        }

    private:
        void init_oscillators(const nt::SampleRate &sample_rate, const uint8_t waveform,
                              const nt::OscillatorFrequency &frequency, const nt::DetuneHz &detune)
        {
            for (auto &osc : oscillators)
            {
                osc.oscillator.Init(sample_rate.get());
                osc.oscillator.SetWaveform(waveform);
                osc.oscillator.SetFreq(frequency.get());
            }

            compute_detune(detune);
        }

        float process_oscillators()
        {
            float osc_value{0.f};
            for (auto &osc : oscillators)
            {
                osc.oscillator.SetFreq(current_frequency.get() + osc.detune_amount);
                osc_value += osc.oscillator.Process();
            }
            return osc_value;
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
            float detune_amount;
        };

        State state{PENDING_TRANSIT_TO_TARGET};
        nt::OscillatorFrequency start_frequency{0.f};
        nt::OscillatorFrequency target_frequency{0.f};
        nt::OscillatorFrequency current_frequency{0.f};
        DetunedOscillator oscillators[num_oscillators];
        nt::OscillatorFrequency base_animation_frequency{0.f};
        daisysp::Oscillator animation_lfo;
        Scaler animation_scaler;
    };

} // namespace deepnote
