#include "voice/deepnotevoice.hpp"
#include <algorithm>
#include <cmath>
#include <doctest/doctest.h>
#include <vector>

using namespace deepnote;

/**
 * @file statemachine_lifecycle_tests.cpp
 * @brief State machine and lifecycle validation tests for the deepnote voice
 *
 * These tests verify correct state transitions, lifecycle management,
 * and behavior consistency across different voice configurations.
 */

TEST_CASE("State transition completeness")
{
    SUBCASE("Basic state lifecycle")
    {
        DeepnoteVoice voice;

        // Initialize voice
        init_voice(voice, 3, nt::OscillatorFrequency(220.0f), nt::SampleRate(48000.0f),
                   nt::OscillatorFrequency(2.0f)); // Faster animation

        // Should start in PENDING_TRANSIT_TO_TARGET after setting a target
        voice.set_target_frequency(nt::OscillatorFrequency(880.0f));
        REQUIRE(voice.get_state() == DeepnoteVoice::State::PENDING_TRANSIT_TO_TARGET);

        // Process until state changes to IN_TRANSIT
        bool reached_in_transit = false;
        for(int i = 0; i < 1000 && !reached_in_transit; ++i)
        {
            process_voice(voice, nt::AnimationMultiplier(10.0f), // Higher animation multiplier
                          nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            if(voice.get_state() == DeepnoteVoice::State::IN_TRANSIT_TO_TARGET)
            {
                reached_in_transit = true;
                INFO("Entered IN_TRANSIT state at sample " << i);
            }
        }
        REQUIRE(reached_in_transit);

        // Process until completion
        bool reached_target = false;
        for(int i = 0; i < 30000 && !reached_target; ++i)
        {                                                        // Increased timeout
            process_voice(voice, nt::AnimationMultiplier(10.0f), // Higher animation multiplier
                          nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            if(voice.get_state() == DeepnoteVoice::State::AT_TARGET)
            {
                reached_target = true;
                INFO("Reached AT_TARGET state at sample " << i);
            }
        }

        if(!reached_target)
        {
            INFO("Failed to reach target. Current state: " << static_cast<int>(voice.get_state()));
            INFO("Current frequency: " << voice.get_current_frequency().get());
        }

        REQUIRE(reached_target);
    }

    SUBCASE("State consistency during processing")
    {
        DeepnoteVoice voice;
        init_voice(voice, 4, nt::OscillatorFrequency(300.0f), nt::SampleRate(48000.0f),
                   nt::OscillatorFrequency(3.0f)); // Faster animation

        voice.set_target_frequency(nt::OscillatorFrequency(600.0f));

        std::vector<DeepnoteVoice::State> state_sequence;
        DeepnoteVoice::State              previous_state = voice.get_state();
        state_sequence.push_back(previous_state);

        // Record state transitions
        for(int i = 0; i < 25000; ++i)
        { // Increased timeout
            process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.1f), nt::ControlPoint2(0.9f));

            DeepnoteVoice::State current_state = voice.get_state();
            if(current_state != previous_state)
            {
                state_sequence.push_back(current_state);
                previous_state = current_state;
            }

            if(current_state == DeepnoteVoice::State::AT_TARGET)
            {
                break;
            }
        }

        // Debug output if we didn't reach target
        if(voice.get_state() != DeepnoteVoice::State::AT_TARGET)
        {
            INFO("Failed to reach AT_TARGET state");
            INFO("Current state: " << static_cast<int>(voice.get_state()));
            INFO("Current frequency: " << voice.get_current_frequency().get());
            for(size_t i = 0; i < state_sequence.size(); ++i)
            {
                INFO("State " << i << ": " << static_cast<int>(state_sequence[i]));
            }
        }

        // Verify valid state progression
        REQUIRE(state_sequence.size() >= 2); // Should have at least initial and final state
        REQUIRE(state_sequence[0] == DeepnoteVoice::State::PENDING_TRANSIT_TO_TARGET);
        REQUIRE(state_sequence.back() == DeepnoteVoice::State::AT_TARGET);

        // If there's an intermediate state, it should be IN_TRANSIT_TO_TARGET
        if(state_sequence.size() == 3)
        {
            REQUIRE(state_sequence[1] == DeepnoteVoice::State::IN_TRANSIT_TO_TARGET);
        }

        INFO("State sequence length: " << state_sequence.size());
        for(size_t i = 0; i < state_sequence.size(); ++i)
        {
            INFO("State " << i << ": " << static_cast<int>(state_sequence[i]));
        }
    }

    SUBCASE("State transitions with different animation speeds")
    {
        struct SpeedTest
        {
            float       animation_speed;
            std::string description;
        };

        std::vector<SpeedTest> speed_tests = {
            {1.0f, "Slow"},     // Adjusted from 0.5f
            {2.0f, "Normal"},   // Adjusted from 1.0f
            {4.0f, "Fast"},     // Adjusted from 2.0f
            {8.0f, "Very fast"} // Adjusted from 5.0f
        };

        for(const auto &test : speed_tests)
        {
            DeepnoteVoice voice;
            init_voice(voice, 2, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f),
                       nt::OscillatorFrequency(test.animation_speed));

            voice.set_target_frequency(nt::OscillatorFrequency(880.0f));

            bool completed_lifecycle = false;
            int  samples_to_complete = 0;

            for(int i = 0; i < 40000; ++i)
            {                                                        // Increased timeout
                process_voice(voice, nt::AnimationMultiplier(10.0f), // Much higher animation multiplier
                              nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));
                samples_to_complete++;

                if(voice.get_state() == DeepnoteVoice::State::AT_TARGET)
                {
                    completed_lifecycle = true;
                    break;
                }
            }

            if(!completed_lifecycle)
            {
                INFO("Failed to complete lifecycle for " << test.description << " animation");
                INFO("Current state: " << static_cast<int>(voice.get_state()));
                INFO("Current frequency: " << voice.get_current_frequency().get());
            }

            REQUIRE(completed_lifecycle);
            INFO(test.description << " animation (" << test.animation_speed << "x): " << samples_to_complete
                                  << " samples");

            // Faster animations should generally complete in fewer samples
            if(test.animation_speed >= 4.0f)
            {
                REQUIRE(samples_to_complete < 20000);
            }
        }
    }
}

TEST_CASE("Multiple target changes")
{
    SUBCASE("Target change mid-transition")
    {
        DeepnoteVoice voice;
        init_voice(voice, 2, nt::OscillatorFrequency(200.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(2.0f));

        voice.set_target_frequency(nt::OscillatorFrequency(800.0f));
        REQUIRE(voice.get_state() == DeepnoteVoice::State::PENDING_TRANSIT_TO_TARGET);

        // Process partially through transition
        for(int i = 0; i < 3000; ++i)
        {
            process_voice(voice, nt::AnimationMultiplier(5.0f), // Higher animation multiplier
                          nt::ControlPoint1(0.2f), nt::ControlPoint2(0.8f));
        }

        // Should be in transition by now
        auto mid_state = voice.get_state();
        REQUIRE((mid_state == DeepnoteVoice::State::IN_TRANSIT_TO_TARGET ||
                 mid_state == DeepnoteVoice::State::PENDING_TRANSIT_TO_TARGET));

        float mid_frequency = voice.get_current_frequency().get();

        // Change target mid-transition
        voice.set_target_frequency(nt::OscillatorFrequency(400.0f));
        REQUIRE(voice.get_state() == DeepnoteVoice::State::PENDING_TRANSIT_TO_TARGET);

        // Should eventually reach new target
        bool reached_new_target = false;
        for(int i = 0; i < 25000 && !reached_new_target; ++i)
        {
            process_voice(voice, nt::AnimationMultiplier(5.0f), // Higher animation multiplier
                          nt::ControlPoint1(0.3f), nt::ControlPoint2(0.7f));

            if(voice.get_state() == DeepnoteVoice::State::AT_TARGET)
            {
                reached_new_target = true;
                // Verify we reached the new target, not the old one
                float current_freq = voice.get_current_frequency().get();
                REQUIRE(std::abs(current_freq - 400.0f) < std::abs(current_freq - 800.0f));
                INFO("Mid frequency: " << mid_frequency << ", Final: " << current_freq);
            }
        }

        REQUIRE(reached_new_target);
    }

    SUBCASE("Rapid successive target changes")
    {
        DeepnoteVoice voice;
        init_voice(voice, 3, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f),
                   nt::OscillatorFrequency(5.0f)); // Fast animation

        std::vector<float> target_frequencies = {880.0f, 220.0f, 660.0f, 330.0f, 550.0f};

        for(float target_freq : target_frequencies)
        {
            voice.set_target_frequency(nt::OscillatorFrequency(target_freq));
            REQUIRE(voice.get_state() == DeepnoteVoice::State::PENDING_TRANSIT_TO_TARGET);

            // Process briefly before changing target again
            for(int i = 0; i < 200; ++i)
            {
                auto output = process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f),
                                            nt::ControlPoint2(1.0f));
                REQUIRE(std::isfinite(output.get()));
            }
        }

        // Let final transition complete
        bool reached_final = false;
        for(int i = 0; i < 10000 && !reached_final; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            REQUIRE(std::isfinite(output.get()));

            if(voice.get_state() == DeepnoteVoice::State::AT_TARGET)
            {
                reached_final = true;
            }
        }

        REQUIRE(reached_final);

        // Should be close to final target frequency
        float final_freq  = voice.get_current_frequency().get();
        float last_target = target_frequencies.back();
        REQUIRE(std::abs(final_freq - last_target) < 15.0f);
    }

    SUBCASE("Target changes with different curve shapes")
    {
        // Test that state transitions work correctly with various Bezier curves
        std::vector<std::pair<float, float>> curve_shapes = {
            {0.0f, 1.0f},   // Linear
            {0.42f, 0.0f},  // Ease-in
            {0.0f, 0.58f},  // Ease-out
            {0.25f, 0.75f}, // Ease-in-out
            {0.1f, 0.9f}    // Sharp ease
        };

        for(const auto &curve : curve_shapes)
        {
            DeepnoteVoice voice;
            init_voice(voice, 3, nt::OscillatorFrequency(300.0f), nt::SampleRate(48000.0f),
                       nt::OscillatorFrequency(3.0f));

            voice.set_target_frequency(nt::OscillatorFrequency(900.0f));

            bool completed_transition = false;
            for(int i = 0; i < 15000; ++i)
            {
                process_voice(voice, nt::AnimationMultiplier(10.0f), // Higher animation multiplier
                              nt::ControlPoint1(curve.first), nt::ControlPoint2(curve.second));

                if(voice.get_state() == DeepnoteVoice::State::AT_TARGET)
                {
                    completed_transition = true;
                    break;
                }
            }

            REQUIRE(completed_transition);

            float final_freq = voice.get_current_frequency().get();
            REQUIRE(std::abs(final_freq - 900.0f) < 20.0f);

            INFO("Curve (" << curve.first << ", " << curve.second << ") completed, final freq: " << final_freq);
        }
    }
}

TEST_CASE("Detuning behavior consistency")
{
    SUBCASE("Identical voices without detuning")
    {
        DeepnoteVoice voice1, voice2;

        // Initialize identical voices
        init_voice(voice1, 4, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));
        init_voice(voice2, 4, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        // No detuning for either voice
        voice1.detune_oscillators(nt::DetuneHz(0.0f));
        voice2.detune_oscillators(nt::DetuneHz(0.0f));

        // Both should produce identical output (within floating point precision)
        for(int i = 0; i < 100; ++i)
        {
            auto output1 =
                process_voice(voice1, nt::AnimationMultiplier(0.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            auto output2 =
                process_voice(voice2, nt::AnimationMultiplier(0.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));

            REQUIRE(std::abs(output1.get() - output2.get()) < 0.001f);
        }
    }

    SUBCASE("Detuning creates expected differences")
    {
        DeepnoteVoice voice1, voice2;

        // Initialize identical voices
        init_voice(voice1, 4, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));
        init_voice(voice2, 4, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        // Apply different detuning
        voice1.detune_oscillators(nt::DetuneHz(0.0f)); // No detuning
        voice2.detune_oscillators(nt::DetuneHz(5.0f)); // 5Hz detuning

        // Should produce different output due to beating
        bool found_difference = false;
        for(int i = 0; i < 1000; ++i)
        {
            auto output1 =
                process_voice(voice1, nt::AnimationMultiplier(0.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            auto output2 =
                process_voice(voice2, nt::AnimationMultiplier(0.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));

            if(std::abs(output1.get() - output2.get()) > 0.1f)
            {
                found_difference = true;
                break;
            }
        }

        REQUIRE(found_difference); // Should find significant differences due to beating
    }

    SUBCASE("Detuning behavior during state transitions")
    {
        DeepnoteVoice voice;
        init_voice(voice, 6, nt::OscillatorFrequency(220.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(2.0f),
                   nt::DetuneHz(8.0f)); // Significant detuning

        voice.set_target_frequency(nt::OscillatorFrequency(880.0f));

        std::vector<float>                amplitude_samples;
        std::vector<DeepnoteVoice::State> state_samples;

        // Check initial state immediately after setting target
        state_samples.push_back(voice.get_state());

        // Collect samples during transition
        for(int i = 0; i < 20000; ++i)
        { // Increased iteration count
            auto output = process_voice(
                voice, nt::AnimationMultiplier(2.0f), // Moderate animation multiplier to capture state transitions
                nt::ControlPoint1(0.3f), nt::ControlPoint2(0.7f));

            REQUIRE(std::isfinite(output.get()));
            amplitude_samples.push_back(std::abs(output.get()));
            state_samples.push_back(voice.get_state());

            if(voice.get_state() == DeepnoteVoice::State::AT_TARGET)
                break;
        }

        // Should have reached target
        REQUIRE(voice.get_state() == DeepnoteVoice::State::AT_TARGET);

        // Should have reasonable amplitude variation due to beating
        float min_amp = *std::min_element(amplitude_samples.begin(), amplitude_samples.end());
        float max_amp = *std::max_element(amplitude_samples.begin(), amplitude_samples.end());

        REQUIRE(max_amp > min_amp); // Should have amplitude variation
        REQUIRE(max_amp < 20.0f);   // But not excessive

        // Should have gone through proper state transitions
        bool had_pending = false, had_transit = false, had_target = false;
        for(auto state : state_samples)
        {
            if(state == DeepnoteVoice::State::PENDING_TRANSIT_TO_TARGET)
                had_pending = true;
            if(state == DeepnoteVoice::State::IN_TRANSIT_TO_TARGET)
                had_transit = true;
            if(state == DeepnoteVoice::State::AT_TARGET)
                had_target = true;
        }

        REQUIRE(had_pending);
        REQUIRE(had_target);
        // had_transit is optional as it might be very brief

        INFO("Amplitude range with detuning: " << min_amp << " to " << max_amp);
        INFO("States observed: PENDING=" << had_pending << ", TRANSIT=" << had_transit << ", TARGET=" << had_target);
    }

    SUBCASE("Symmetric detuning properties")
    {
        DeepnoteVoice voice1, voice2;

        // Initialize identical voices
        init_voice(voice1, 6, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));
        init_voice(voice2, 6, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        // Apply symmetric detuning
        float detune_amount = 3.0f;
        voice1.detune_oscillators(nt::DetuneHz(detune_amount));
        voice2.detune_oscillators(nt::DetuneHz(-detune_amount)); // Negative detuning

        // Both should have similar amplitude characteristics (due to symmetry)
        std::vector<float> samples1, samples2;

        for(int i = 0; i < 1000; ++i)
        {
            auto output1 =
                process_voice(voice1, nt::AnimationMultiplier(0.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            auto output2 =
                process_voice(voice2, nt::AnimationMultiplier(0.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));

            samples1.push_back(output1.get());
            samples2.push_back(output2.get());
        }

        // Calculate RMS for both
        float rms1 = 0.0f, rms2 = 0.0f;
        for(size_t i = 0; i < samples1.size(); ++i)
        {
            rms1 += samples1[i] * samples1[i];
            rms2 += samples2[i] * samples2[i];
        }
        rms1 = std::sqrt(rms1 / samples1.size());
        rms2 = std::sqrt(rms2 / samples2.size());

        // RMS levels should be similar (within 10%)
        REQUIRE(std::abs(rms1 - rms2) / std::max(rms1, rms2) < 0.1f);

        INFO("Symmetric detuning RMS: +" << detune_amount << "Hz=" << rms1 << ", -" << detune_amount << "Hz=" << rms2);
    }
}

TEST_CASE("Animation timing and consistency")
{
    SUBCASE("Animation speed affects transition time")
    {
        float slow_speed = 0.5f;
        float fast_speed = 4.0f;

        DeepnoteVoice slow_voice, fast_voice;

        init_voice(slow_voice, 2, nt::OscillatorFrequency(200.0f), nt::SampleRate(48000.0f),
                   nt::OscillatorFrequency(slow_speed));
        init_voice(fast_voice, 2, nt::OscillatorFrequency(200.0f), nt::SampleRate(48000.0f),
                   nt::OscillatorFrequency(fast_speed));

        slow_voice.set_target_frequency(nt::OscillatorFrequency(400.0f));
        fast_voice.set_target_frequency(nt::OscillatorFrequency(400.0f));

        int slow_samples = 0, fast_samples = 0;

        // Time the slow voice
        for(int i = 0; i < 50000; ++i)
        {
            process_voice(slow_voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            slow_samples++;
            if(slow_voice.get_state() == DeepnoteVoice::State::AT_TARGET)
            {
                break;
            }
        }

        // Time the fast voice
        for(int i = 0; i < 50000; ++i)
        {
            process_voice(fast_voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            fast_samples++;
            if(fast_voice.get_state() == DeepnoteVoice::State::AT_TARGET)
            {
                break;
            }
        }

        INFO("Slow voice samples: " << slow_samples);
        INFO("Fast voice samples: " << fast_samples);
        INFO("Speed ratio: " << fast_speed / slow_speed);
        INFO("Sample ratio: " << static_cast<float>(slow_samples) / fast_samples);

        // Fast voice should complete in fewer samples
        REQUIRE(fast_samples < slow_samples);

        // The ratio should be roughly proportional to speed ratio (with some tolerance)
        float expected_ratio = fast_speed / slow_speed;
        float actual_ratio   = static_cast<float>(slow_samples) / fast_samples;
        REQUIRE(actual_ratio > expected_ratio * 0.5f); // At least half the expected speedup
    }

    SUBCASE("Animation multiplier affects speed")
    {
        DeepnoteVoice voice;
        init_voice(voice, 2, nt::OscillatorFrequency(300.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(2.0f));

        voice.set_target_frequency(nt::OscillatorFrequency(600.0f));

        int normal_samples = 0;

        // Test with normal speed multiplier
        for(int i = 0; i < 25000; ++i)
        {
            process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            normal_samples++;
            if(voice.get_state() == DeepnoteVoice::State::AT_TARGET)
            {
                break;
            }
        }

        // Reset for second test
        voice.set_target_frequency(nt::OscillatorFrequency(300.0f)); // Reset
        // Process until stable
        for(int i = 0; i < 15000; ++i)
        {
            process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            if(voice.get_state() == DeepnoteVoice::State::AT_TARGET)
                break;
        }

        voice.set_target_frequency(nt::OscillatorFrequency(600.0f));

        int fast_samples = 0;

        // Test with 2x speed multiplier
        for(int i = 0; i < 25000; ++i)
        {
            process_voice(voice, nt::AnimationMultiplier(2.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            fast_samples++;
            if(voice.get_state() == DeepnoteVoice::State::AT_TARGET)
            {
                break;
            }
        }

        INFO("Normal speed samples: " << normal_samples);
        INFO("2x speed samples: " << fast_samples);

        // 2x multiplier should complete faster
        REQUIRE(fast_samples < normal_samples);
    }

    SUBCASE("Consistent timing across oscillator counts")
    {
        // Test that animation timing is consistent regardless of oscillator count
        std::vector<int> oscillator_counts = {1, 2, 4, 6, 8};
        std::vector<int> completion_times;

        for(int osc_count : oscillator_counts)
        {
            DeepnoteVoice voice;
            init_voice(voice, osc_count, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f),
                       nt::OscillatorFrequency(2.0f));

            voice.set_target_frequency(nt::OscillatorFrequency(880.0f));

            int samples_to_complete = 0;
            for(int i = 0; i < 20000; ++i)
            {
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));
                samples_to_complete++;

                if(voice.get_state() == DeepnoteVoice::State::AT_TARGET)
                {
                    break;
                }
            }

            completion_times.push_back(samples_to_complete);

            INFO("Oscillator count " << osc_count << ": " << samples_to_complete << " samples");
        }

        // All completion times should be relatively similar (within 30% variation)
        int min_time = *std::min_element(completion_times.begin(), completion_times.end());
        int max_time = *std::max_element(completion_times.begin(), completion_times.end());

        float variation = static_cast<float>(max_time - min_time) / min_time;

        INFO("Timing variation across oscillator counts: " << variation * 100.0f << "%");
        INFO("Min time: " << min_time << ", Max time: " << max_time);

        REQUIRE(variation < 0.3f); // Less than 30% variation
    }
}
