#pragma once

#include "voice/frequencytable.hpp"
#include "ranges/range.hpp"
#include "ranges/scaler.hpp"
#include "unitshapers/bezier.hpp"
#include "oscfrequency.hpp"
#include "Synthesis/oscillator.h"
#include <array>
#include <stdexcept>

namespace deepnote
{
    namespace constants {
        static constexpr float DEFAULT_LFO_AMPLITUDE = 0.5f;
        static constexpr float DEFAULT_DETUNE_HZ = 2.5f;
        static constexpr float TARGET_FREQUENCY_TOLERANCE = 1.0f;
        static constexpr size_t NEAR_BEGINNING_SAMPLES = 4800;
    }

    namespace nt
    {
        using SampleRate = NamedType<float, struct SampleRateTag>;
        using AnimationMultiplier = NamedType<float, struct AnimationMultiplierTag>;
        using DetuneHz = NamedType<float, struct DetuneHzTag>;
        using OscillatorFrequencyRange = NamedType<Range, struct OscillatorFrequencyRangeTag>;
        using OscillatorValue = NamedType<float, struct OscillatorValueTag>;
    }

    struct NoopTrace
    {
        template <typename T, typename... Args>
        void operator()(T first, Args... rest) const 
        {}

        template <typename T>
        void operator()(T value) const 
        {}
    };

    struct DeepnoteVoice
    {
        static constexpr size_t MAX_OSCILLATORS = 16;
        const float LFO_AMPLITUDE{constants::DEFAULT_LFO_AMPLITUDE};

        enum State
        {
            PENDING_TRANSIT_TO_TARGET,
            IN_TRANSIT_TO_TARGET,
            AT_TARGET
        };

        DeepnoteVoice() = default;
        virtual ~DeepnoteVoice() = default;

        nt::OscillatorFrequency get_target_frequency() const noexcept
        {
            return target_frequency;
        }

        void set_target_frequency(const nt::OscillatorFrequency freq)
        {
            if (freq.get() < 0.0f) {
                throw std::invalid_argument("Target frequency must be non-negative");
            }
            
            //  set up a new transit from something close to the current frequency of
            //  the voice and the new target frequency
            this->start_frequency = this->current_frequency;
            this->target_frequency = freq;
            this->state = PENDING_TRANSIT_TO_TARGET;
        }

        nt::OscillatorFrequency get_start_frequency() const noexcept
        {
            return start_frequency;
        }

        void set_start_frequency(const nt::OscillatorFrequency freq)
        {
            if (freq.get() < 0.0f) {
                throw std::invalid_argument("Start frequency must be non-negative");
            }
            
            //  set up a new transit from a new start frequency
            this->start_frequency = freq;
            this->current_frequency = this->start_frequency;
            this->state = PENDING_TRANSIT_TO_TARGET;
        }

        nt::OscillatorFrequency get_current_frequency() const noexcept
        {
            return current_frequency;
        }

        void set_current_frequency(const nt::OscillatorFrequency freq) noexcept
        {
            this->current_frequency = freq;
        }

        void scale_lfo_base_freq(const nt::AnimationMultiplier mulitplier)
        {
            if (mulitplier.get() < 0.0f) {
                throw std::invalid_argument("Animation multiplier must be non-negative");
            }
            
            lfo.SetFreq(lfo_base_freq.get() * mulitplier.get());
        }

        nt::OscillatorFrequency get_lfo_base_freq() const noexcept
        {
            return lfo_base_freq;
        }

        void set_lfo_base_freq(const nt::OscillatorFrequency freq) noexcept
        {
            this->lfo_base_freq = freq;
        }

        bool is_at_target() const noexcept
        {
            return state == AT_TARGET;
        }

        State get_state() const noexcept
        {
            return state;
        }

        void set_state(const State state) noexcept
        {
            this->state = state;
        }

        void init_lfo(const nt::SampleRate sample_rate, const nt::OscillatorFrequency base_freq)
        {
            if (sample_rate.get() <= 0.0f) {
                throw std::invalid_argument("Sample rate must be positive");
            }
            if (base_freq.get() < 0.0f) {
                throw std::invalid_argument("LFO base frequency must be non-negative");
            }
            
            lfo_base_freq = base_freq;
            lfo.Init(sample_rate.get());
            lfo.SetAmp(LFO_AMPLITUDE);
            lfo.SetWaveform(daisysp::Oscillator::WAVE_RAMP);
            lfo.SetFreq(lfo_base_freq.get());
        }

        nt::OscillatorValue process_lfo()
        {
            return nt::OscillatorValue(lfo.Process() + LFO_AMPLITUDE);
        }

        void reset_lfo() noexcept
        {
            lfo.Reset();
        }

        void init_oscillators(const size_t count, nt::SampleRate sample_rate, nt::OscillatorFrequency start_frequency)
        {
            if (count == 0) {
                throw std::invalid_argument("Oscillator count must be at least 1");
            }
            if (count > MAX_OSCILLATORS) {
                throw std::invalid_argument("Oscillator count exceeds maximum of " + std::to_string(MAX_OSCILLATORS));
            }
            if (sample_rate.get() <= 0.0f) {
                throw std::invalid_argument("Sample rate must be positive");
            }
            if (start_frequency.get() < 0.0f) {
                throw std::invalid_argument("Start frequency must be non-negative");
            }
            
            oscillator_count = count;
            for (size_t i = 0; i < oscillator_count; ++i)
            {
                oscillators[i].oscillator.Init(sample_rate.get());
                oscillators[i].oscillator.SetWaveform(daisysp::Oscillator::WAVE_POLYBLEP_SAW);
                oscillators[i].oscillator.SetFreq(start_frequency.get());
                oscillators[i].detune_amount = 0.f;
            }
        }

        void detune_oscillators(const nt::DetuneHz detune)
        {
            // If we only have one oscillator, we don't need to detune it
            // Otherwise distribute the either side of the fundamental frequency by an
            // integer muliples of detune.
            const auto half = oscillator_count / 2;
            for (size_t i = 0; i < oscillator_count; ++i)
            {
                if (oscillator_count <= 1)
                {
                    oscillators[i].detune_amount = 0.f;
                }
                else
                {
                    const int8_t idx = i - half + ((i >= half) ? 1 : 0);
                    oscillators[i].detune_amount = idx * detune.get();
                }
            }
        }

        nt::OscillatorValue process_oscillators()
        {
            float osc_value{0.f};
            for (size_t i = 0; i < oscillator_count; ++i)
            {
                oscillators[i].oscillator.SetFreq(current_frequency.get() + oscillators[i].detune_amount);
                osc_value += oscillators[i].oscillator.Process();
            }
            return nt::OscillatorValue(osc_value);
        }

    private:
        struct DetunedOscillator
        {
            daisysp::Oscillator oscillator;
            float detune_amount;
        };

        State state{PENDING_TRANSIT_TO_TARGET};
        nt::OscillatorFrequency start_frequency{0.f};
        nt::OscillatorFrequency target_frequency{0.f};
        nt::OscillatorFrequency current_frequency{0.f};
        std::array<DetunedOscillator, MAX_OSCILLATORS> oscillators{};
        size_t oscillator_count{0};
        nt::OscillatorFrequency lfo_base_freq{0.f};
        daisysp::Oscillator lfo;
    };


    void init_voice(DeepnoteVoice &voice, const size_t oscillator_count, 
                    const nt::OscillatorFrequency start_frequency, const nt::SampleRate sample_rate, 
                    const nt::OscillatorFrequency lfo_frequency, const nt::DetuneHz detune = nt::DetuneHz(constants::DEFAULT_DETUNE_HZ))
    {
        voice.set_start_frequency(start_frequency);
        voice.set_current_frequency(start_frequency);
        voice.set_target_frequency(start_frequency);
        voice.set_state(voice.PENDING_TRANSIT_TO_TARGET);
        voice.init_lfo(sample_rate, lfo_frequency);
        voice.init_oscillators(oscillator_count, sample_rate, start_frequency);
        voice.detune_oscillators(detune);
    }


    namespace {
        nt::OscillatorFrequency calculate_shaped_frequency(
            DeepnoteVoice& voice,
            const nt::AnimationMultiplier lfo_multiplier,
            const nt::ControlPoint1 cp1, 
            const nt::ControlPoint2 cp2) {
            
            voice.scale_lfo_base_freq(lfo_multiplier);
            const auto raw_lfo_value = voice.process_lfo();
            auto shaped_lfo_value = nt::OscillatorValue(BezierUnitShaper(cp1, cp2)(raw_lfo_value.get()));

            const auto start_frequency = voice.get_start_frequency();
            const auto target_frequency = voice.get_target_frequency();

            //  If we want the frequency to decrease we need to flip the shaped value
            if (start_frequency.get() > target_frequency.get())
            {
                shaped_lfo_value = nt::OscillatorValue(1.f - shaped_lfo_value.get());
            }

            //  Scale the 0.0 to 1.0 shaped value to the start and target frequency range
            const auto animationScaler = Scaler(
                nt::InputRange(Range(
                    nt::RangeLow(0.f),
                    nt::RangeHigh(1.f))),
                nt::OutputRange(Range(
                    nt::RangeLow(start_frequency.get()),
                    nt::RangeHigh(target_frequency.get()))));

            return nt::OscillatorFrequency(animationScaler(shaped_lfo_value.get()));
        }

        DeepnoteVoice::State update_voice_state(
            DeepnoteVoice& voice,
            const nt::OscillatorFrequency current_frequency) {
            
            auto state = voice.get_state();
            const auto start_frequency = voice.get_start_frequency();
            const auto target_frequency = voice.get_target_frequency();

            // Valid frequencies are from start_frequency to target_frequency
            const nt::OscillatorFrequencyRange validFrequencyRange(
                Range(
                    nt::RangeLow(start_frequency.get()),
                    nt::RangeHigh(target_frequency.get())));

            if (validFrequencyRange.get().contains(current_frequency.get()))
            {
                //  The frequency is within the valid range, so check to see if we've
                //  reached the start or target frequency
                if (state == voice.IN_TRANSIT_TO_TARGET)
                {
                    const nt::OscillatorFrequencyRange targetRange(
                        Range(
                            nt::RangeLow(target_frequency.get() - constants::TARGET_FREQUENCY_TOLERANCE),
                            nt::RangeHigh(target_frequency.get() + constants::TARGET_FREQUENCY_TOLERANCE)));

                    if (targetRange.get().contains(current_frequency.get()))
                    {
                        state = voice.AT_TARGET;
                    }
                }
            }
            else
            {
                // The frequency is outside the valid range, so constrain it
                state = (state == voice.IN_TRANSIT_TO_TARGET) ? voice.AT_TARGET : state;
            }

            return state;
        }

        nt::OscillatorFrequency constrain_frequency(
            const DeepnoteVoice& voice,
            const nt::OscillatorFrequency frequency) {
            
            const auto start_frequency = voice.get_start_frequency();
            const auto target_frequency = voice.get_target_frequency();

            const nt::OscillatorFrequencyRange validFrequencyRange(
                Range(
                    nt::RangeLow(start_frequency.get()),
                    nt::RangeHigh(target_frequency.get())));

            return nt::OscillatorFrequency(validFrequencyRange.get().constrain(frequency.get()));
        }
    }


    template <typename TraceFunc = NoopTrace>
    nt::OscillatorValue process_voice(DeepnoteVoice &voice, const nt::AnimationMultiplier lfo_multiplier,
                                      const nt::ControlPoint1 cp1, const nt::ControlPoint2 cp2, 
                                      const TraceFunc &trace_functor = NoopTrace())
    {
        nt::OscillatorFrequency unconstrained_freq(0.f); // only used for tracing
        const auto in_state{voice.get_state()};
        auto state = in_state;

        //  if we in a pending state, reset the animation LFO and move to the next state
        if (state == voice.PENDING_TRANSIT_TO_TARGET)
        {
            voice.reset_lfo();
            state = voice.IN_TRANSIT_TO_TARGET;
        }

        const auto start_frequency = voice.get_start_frequency();
        const auto target_frequency = voice.get_target_frequency();
        auto current_frequency = voice.get_current_frequency();

        if (state == voice.AT_TARGET)
        {
            current_frequency = target_frequency;
        }
        else
        {
            current_frequency = calculate_shaped_frequency(voice, lfo_multiplier, cp1, cp2);
            unconstrained_freq = current_frequency;
            state = update_voice_state(voice, current_frequency);
            current_frequency = constrain_frequency(voice, current_frequency);
            
            // If we reached target after constraining, set exact target frequency
            if (state == voice.AT_TARGET) {
                current_frequency = target_frequency;
            }
        }

        voice.set_current_frequency(current_frequency);
        voice.set_state(state);

        //  Update all oscillators using the new frequency
        nt::OscillatorValue osc_value = voice.process_oscillators();

        //  Give the traceFunctor a chance to log the state of the voice
        trace_functor(
            start_frequency.get(),
            target_frequency.get(),
            in_state,
            state,
            0.0f, // raw_lfo_value - simplified for now
            0.0f, // shaped_lfo_value - simplified for now
            unconstrained_freq.get(),
            current_frequency.get(),
            osc_value.get());

        return osc_value;
    }
} // namespace deepnote
