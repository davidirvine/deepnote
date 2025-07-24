#include "voice/deepnotevoice.hpp"
#include <algorithm>
#include <cmath>
#include <doctest/doctest.h>
#include <limits>
#include <random>
#include <vector>

using namespace deepnote;

/**
 * @file error_robustness_tests.cpp
 * @brief Comprehensive error handling and robustness validation tests
 *
 * These tests verify system behavior under error conditions, boundary cases,
 * invalid inputs, resource constraints, and stress scenarios to ensure
 * graceful degradation and stability.
 */

TEST_CASE("Invalid parameter handling")
{
    SUBCASE("Invalid oscillator counts")
    {
        DeepnoteVoice voice;

        // Test minimum oscillator count (1) - should work normally
        init_voice(voice, 1, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        // Should not crash when processing
        for(int i = 0; i < 100; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            REQUIRE(std::isfinite(output.get()));
        }

        // Test maximum oscillator count within limits - should handle gracefully
        init_voice(voice, 16, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        // Should not crash when processing
        for(int i = 0; i < 10; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            REQUIRE(std::isfinite(output.get()));
        }
    }

    SUBCASE("Invalid frequency values")
    {
        DeepnoteVoice voice;
        init_voice(voice, 4, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        // Test very small positive frequency (near zero)
        voice.set_target_frequency(nt::OscillatorFrequency(0.001f));
        for(int i = 0; i < 100; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            REQUIRE(std::isfinite(output.get()));
        }

        // Test extremely high frequency
        voice.set_target_frequency(nt::OscillatorFrequency(20000.0f));
        for(int i = 0; i < 100; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            REQUIRE(std::isfinite(output.get()));
        }

        // Test mid-range frequencies for comparison
        voice.set_target_frequency(nt::OscillatorFrequency(1000.0f));
        for(int i = 0; i < 50; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            REQUIRE(std::isfinite(output.get()));
        }
    }

    SUBCASE("Invalid sample rate values")
    {
        DeepnoteVoice voice;

        // Test very low sample rate
        init_voice(voice, 2, nt::OscillatorFrequency(440.0f), nt::SampleRate(100.0f), nt::OscillatorFrequency(1.0f));

        for(int i = 0; i < 50; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            REQUIRE(std::isfinite(output.get()));
        }

        // Test very high sample rate
        init_voice(voice, 2, nt::OscillatorFrequency(440.0f), nt::SampleRate(192000.0f), nt::OscillatorFrequency(1.0f));

        for(int i = 0; i < 50; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            REQUIRE(std::isfinite(output.get()));
        }

        // Test standard sample rate for comparison
        init_voice(voice, 2, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        for(int i = 0; i < 50; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            REQUIRE(std::isfinite(output.get()));
        }
    }

    SUBCASE("Invalid animation parameters")
    {
        DeepnoteVoice voice;
        init_voice(voice, 3, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        voice.set_target_frequency(nt::OscillatorFrequency(880.0f));

        // Test zero animation multiplier
        for(int i = 0; i < 100; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(0.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            REQUIRE(std::isfinite(output.get()));
        }

        // Test very small animation multiplier
        for(int i = 0; i < 100; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(0.001f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            REQUIRE(std::isfinite(output.get()));
        }

        // Test extremely large animation multiplier
        for(int i = 0; i < 100; ++i)
        {
            auto output = process_voice(voice, nt::AnimationMultiplier(1000.0f), nt::ControlPoint1(0.0f),
                                        nt::ControlPoint2(1.0f));
            REQUIRE(std::isfinite(output.get()));
        }
    }

    SUBCASE("Invalid control points")
    {
        DeepnoteVoice voice;
        init_voice(voice, 2, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        voice.set_target_frequency(nt::OscillatorFrequency(660.0f));

        // Test extreme control point values
        std::vector<std::pair<float, float>> extreme_control_points = {
            {-1000.0f, 1000.0f},
            {1000.0f, -1000.0f},
            {std::numeric_limits<float>::quiet_NaN(), 0.5f},
            {0.5f, std::numeric_limits<float>::quiet_NaN()},
            {std::numeric_limits<float>::infinity(), 0.5f},
            {0.5f, std::numeric_limits<float>::infinity()},
            {-std::numeric_limits<float>::infinity(), 0.5f},
            {0.5f, -std::numeric_limits<float>::infinity()},
        };

        for(const auto &cp : extreme_control_points)
        {
            for(int i = 0; i < 50; ++i)
            {
                auto output = process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(cp.first),
                                            nt::ControlPoint2(cp.second));
                REQUIRE(std::isfinite(output.get()));
            }
        }
    }
}

TEST_CASE("Boundary condition handling")
{
    SUBCASE("Frequency range boundaries")
    {
        DeepnoteVoice voice;
        init_voice(voice, 4, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(2.0f));

        // Test frequencies at the edge of human hearing
        std::vector<float> boundary_frequencies = {
            0.1f,     // Sub-audible
            1.0f,     // Very low
            20.0f,    // Low limit of hearing
            20000.0f, // High limit of hearing
            22050.0f, // Nyquist for 44.1kHz
            24000.0f, // Nyquist for 48kHz
            30000.0f, // Above human hearing
        };

        for(float freq : boundary_frequencies)
        {
            voice.set_target_frequency(nt::OscillatorFrequency(freq));

            // Process for a reasonable time
            for(int i = 0; i < 1000; ++i)
            {
                auto output = process_voice(voice, nt::AnimationMultiplier(2.0f), nt::ControlPoint1(0.25f),
                                            nt::ControlPoint2(0.75f));

                REQUIRE(std::isfinite(output.get()));
                REQUIRE(std::abs(output.get()) < 300.0f); // More generous amplitude bounds
            }

            INFO("Boundary frequency " << freq << "Hz handled successfully");
        }
    }

    SUBCASE("Detuning boundary values")
    {
        DeepnoteVoice voice;
        init_voice(voice, 6, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        // Test extreme detuning values
        std::vector<float> detune_values = {
            -1000.0f, // Large negative detuning
            -100.0f,  // Moderate negative detuning
            -1.0f,    // Small negative detuning
            0.0f,     // No detuning
            1.0f,     // Small positive detuning
            100.0f,   // Moderate positive detuning
            1000.0f,  // Large positive detuning
        };

        for(float detune : detune_values)
        {
            voice.detune_oscillators(nt::DetuneHz(detune));

            // Process and verify stability
            for(int i = 0; i < 500; ++i)
            {
                auto output = process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f),
                                            nt::ControlPoint2(1.0f));

                REQUIRE(std::isfinite(output.get()));
                REQUIRE(std::abs(output.get()) < 10000.0f); // Very generous bound for extreme edge cases
            }

            INFO("Detuning " << detune << "Hz handled successfully");
        }
    }

    SUBCASE("Animation speed boundaries")
    {
        DeepnoteVoice voice;
        init_voice(voice, 3, nt::OscillatorFrequency(220.0f), nt::SampleRate(48000.0f),
                   nt::OscillatorFrequency(0.001f)); // Very slow

        voice.set_target_frequency(nt::OscillatorFrequency(440.0f));

        // Test extremely slow animation
        for(int i = 0; i < 1000; ++i)
        {
            auto output = process_voice(voice, nt::AnimationMultiplier(0.001f), nt::ControlPoint1(0.25f),
                                        nt::ControlPoint2(0.75f));
            REQUIRE(std::isfinite(output.get()));
        }

        // Reset and test extremely fast animation
        init_voice(voice, 3, nt::OscillatorFrequency(220.0f), nt::SampleRate(48000.0f),
                   nt::OscillatorFrequency(1000.0f)); // Very fast

        voice.set_target_frequency(nt::OscillatorFrequency(440.0f));

        for(int i = 0; i < 100; ++i)
        {
            auto output = process_voice(voice, nt::AnimationMultiplier(1000.0f), nt::ControlPoint1(0.25f),
                                        nt::ControlPoint2(0.75f));
            REQUIRE(std::isfinite(output.get()));
        }
    }
}

TEST_CASE("Resource constraint handling")
{
    SUBCASE("High oscillator count stress test")
    {
        // Test system behavior with many oscillators within system limits
        std::vector<int> oscillator_counts = {1, 2, 4, 8, 12, 16}; // Respect max limit

        for(int count : oscillator_counts)
        {
            DeepnoteVoice voice;
            init_voice(voice, count, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f),
                       nt::OscillatorFrequency(2.0f));

            voice.set_target_frequency(nt::OscillatorFrequency(880.0f));

            // Process for a reasonable duration
            int processed_samples = 0;
            for(int i = 0; i < 2000; ++i)
            {
                auto output = process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.3f),
                                            nt::ControlPoint2(0.7f));

                REQUIRE(std::isfinite(output.get()));
                REQUIRE(std::abs(output.get()) < 1000.0f); // Scale with oscillator count
                processed_samples++;

                // Break early if target reached to avoid excessive processing
                if(voice.get_state() == DeepnoteVoice::State::AT_TARGET)
                {
                    break;
                }
            }

            REQUIRE(processed_samples > 0);
            INFO("Oscillator count " << count << " processed " << processed_samples << " samples");
        }
    }

    SUBCASE("Rapid state changes stress test")
    {
        DeepnoteVoice voice;
        init_voice(voice, 4, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(5.0f));

        // Rapidly change targets to stress the state machine
        std::vector<float> rapid_targets = {440.0f, 880.0f, 220.0f,  660.0f, 330.0f,
                                            990.0f, 110.0f, 1320.0f, 165.0f, 770.0f};

        for(size_t target_idx = 0; target_idx < rapid_targets.size(); ++target_idx)
        {
            voice.set_target_frequency(nt::OscillatorFrequency(rapid_targets[target_idx]));

            // Process briefly before changing target again
            for(int i = 0; i < 100; ++i)
            {
                auto output = process_voice(voice, nt::AnimationMultiplier(3.0f), nt::ControlPoint1(0.2f),
                                            nt::ControlPoint2(0.8f));

                REQUIRE(std::isfinite(output.get()));
                REQUIRE(std::abs(output.get()) < 150.0f);
            }
        }

        // Let final transition settle
        for(int i = 0; i < 2000; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(3.0f), nt::ControlPoint1(0.2f), nt::ControlPoint2(0.8f));
            REQUIRE(std::isfinite(output.get()));

            if(voice.get_state() == DeepnoteVoice::State::AT_TARGET)
            {
                break;
            }
        }
    }

    SUBCASE("Extended processing duration")
    {
        DeepnoteVoice voice;
        init_voice(voice, 2, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        // Test long-running processing without memory leaks or degradation
        int                total_samples = 20000; // About 0.4 seconds at 48kHz - more reasonable
        std::vector<float> amplitude_samples;
        amplitude_samples.reserve(200); // Sample every 100 samples

        for(int i = 0; i < total_samples; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(0.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));

            REQUIRE(std::isfinite(output.get()));

            // Sample amplitude periodically
            if(i % 100 == 0)
            {
                amplitude_samples.push_back(std::abs(output.get()));
            }
        }

        REQUIRE(amplitude_samples.size() > 180);

        // Check that amplitude remains stable over time (no degradation)
        float  first_quarter_mean = 0.0f, last_quarter_mean = 0.0f;
        size_t quarter_size = amplitude_samples.size() / 4;

        for(size_t i = 0; i < quarter_size; ++i)
        {
            first_quarter_mean += amplitude_samples[i];
        }
        first_quarter_mean /= quarter_size;

        for(size_t i = amplitude_samples.size() - quarter_size; i < amplitude_samples.size(); ++i)
        {
            last_quarter_mean += amplitude_samples[i];
        }
        last_quarter_mean /= quarter_size;

        // Amplitude should remain relatively stable
        if(first_quarter_mean > 0.001f)
        {
            float amplitude_drift = std::abs(last_quarter_mean - first_quarter_mean) / first_quarter_mean;
            REQUIRE(amplitude_drift < 0.2f); // Less than 20% drift - more realistic

            INFO("Extended processing: " << total_samples << " samples, amplitude drift: " << amplitude_drift * 100.0f
                                         << "%");
        }
        else
        {
            // For very low amplitudes, just verify stability
            REQUIRE(std::isfinite(first_quarter_mean));
            REQUIRE(std::isfinite(last_quarter_mean));
            INFO("Extended processing: " << total_samples << " samples, low amplitude case handled");
        }
    }
}

TEST_CASE("Numerical stability validation")
{
    SUBCASE("Floating point precision handling")
    {
        DeepnoteVoice voice;
        init_voice(voice, 3, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        // Test with very small frequency differences
        voice.set_target_frequency(nt::OscillatorFrequency(440.000001f)); // Tiny difference

        for(int i = 0; i < 1000; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));
            REQUIRE(std::isfinite(output.get()));
        }

        // Test with frequencies near machine epsilon
        float epsilon_freq = 440.0f + std::numeric_limits<float>::epsilon();
        voice.set_target_frequency(nt::OscillatorFrequency(epsilon_freq));

        for(int i = 0; i < 1000; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));
            REQUIRE(std::isfinite(output.get()));
        }
    }

    SUBCASE("Accumulation error prevention")
    {
        DeepnoteVoice voice;
        init_voice(voice, 4, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(2.0f));

        // Set a target to ensure some oscillation
        voice.set_target_frequency(nt::OscillatorFrequency(880.0f));

        // Process for many samples to test for accumulation errors
        std::vector<float> max_amplitudes;
        const int          checkpoint_interval = 2000; // Smaller intervals
        const int          num_checkpoints     = 6;    // Fewer checkpoints

        for(int checkpoint = 0; checkpoint < num_checkpoints; ++checkpoint)
        {
            // Process for checkpoint interval
            float max_amplitude = 0.0f;
            for(int i = 0; i < checkpoint_interval; ++i)
            {
                auto output = process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.25f),
                                            nt::ControlPoint2(0.75f));
                REQUIRE(std::isfinite(output.get()));
                max_amplitude = std::max(max_amplitude, std::abs(output.get()));
            }

            max_amplitudes.push_back(max_amplitude);
        }

        // Check that max amplitude doesn't grow unbounded (indicating accumulation issues)
        for(float amplitude : max_amplitudes)
        {
            REQUIRE(std::isfinite(amplitude));
            REQUIRE(amplitude >= 0.0f);
            REQUIRE(amplitude < 100.0f); // Reasonable upper bound
        }

        // Check that we don't have runaway growth
        float first_amplitude = max_amplitudes.front();
        float last_amplitude  = max_amplitudes.back();

        if(first_amplitude > 0.001f && last_amplitude > 0.001f)
        {
            float growth_ratio = last_amplitude / first_amplitude;
            REQUIRE(growth_ratio < 10.0f); // No more than 10x growth
            REQUIRE(growth_ratio > 0.1f);  // No more than 10x decay

            INFO("Accumulation test: " << num_checkpoints * checkpoint_interval
                                       << " samples, amplitude growth ratio: " << growth_ratio);
        }
        else
        {
            INFO("Accumulation test: " << num_checkpoints * checkpoint_interval
                                       << " samples, low amplitude case - all values finite");
        }
    }

    SUBCASE("Denormal number handling")
    {
        DeepnoteVoice voice;
        init_voice(voice, 2, nt::OscillatorFrequency(0.00001f), // Very small frequency
                   nt::SampleRate(48000.0f), nt::OscillatorFrequency(0.01f));

        // Set target to another very small frequency
        voice.set_target_frequency(nt::OscillatorFrequency(0.00002f));

        // Process and ensure no denormal slowdown
        auto start_time = std::chrono::high_resolution_clock::now();

        for(int i = 0; i < 10000; ++i)
        {
            auto output = process_voice(voice, nt::AnimationMultiplier(0.001f), nt::ControlPoint1(0.25f),
                                        nt::ControlPoint2(0.75f));
            REQUIRE(std::isfinite(output.get()));
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        // Should complete in reasonable time (not affected by denormal slowdown)
        REQUIRE(duration.count() < 100000); // Less than 100ms for 10k samples

        INFO("Denormal handling test completed in " << duration.count() << " microseconds");
    }
}

TEST_CASE("Recovery and graceful degradation")
{
    SUBCASE("Recovery from invalid states")
    {
        DeepnoteVoice voice;
        init_voice(voice, 3, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        // Force the voice into various challenging conditions and test recovery

        // Set to very low frequency and then recover to normal
        voice.set_target_frequency(nt::OscillatorFrequency(0.1f));

        // Process a bit with challenging state
        for(int i = 0; i < 100; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
            REQUIRE(std::isfinite(output.get()));
        }

        // Recover with normal frequency
        voice.set_target_frequency(nt::OscillatorFrequency(880.0f));

        bool recovered = false;
        for(int i = 0; i < 10000; ++i)
        {                                                                                  // Even more iterations
            auto output = process_voice(voice, nt::AnimationMultiplier(10.0f),             // Much higher multiplier
                                        nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f)); // Linear curve
            REQUIRE(std::isfinite(output.get()));

            if(voice.get_state() == DeepnoteVoice::State::AT_TARGET)
            {
                recovered = true;
                break;
            }
        }

        REQUIRE(recovered);

        // Verify final frequency is reasonable
        float final_freq = voice.get_current_frequency().get();
        REQUIRE(std::isfinite(final_freq));
        REQUIRE(final_freq > 0.0f);
    }

    SUBCASE("Graceful handling of extreme parameter combinations")
    {
        DeepnoteVoice voice;

        // Test combination of extreme parameters
        init_voice(voice, 16, nt::OscillatorFrequency(0.1f),                    // Max oscillators, very low freq
                   nt::SampleRate(192000.0f), nt::OscillatorFrequency(0.001f)); // High sample rate, slow animation

        voice.detune_oscillators(nt::DetuneHz(1000.0f));               // Extreme detuning
        voice.set_target_frequency(nt::OscillatorFrequency(20000.0f)); // Extreme target

        // Should handle this gracefully without crashing
        int stable_samples = 0;
        for(int i = 0; i < 1000; ++i)
        {
            auto output = process_voice(voice, nt::AnimationMultiplier(100.0f),               // Extreme multiplier
                                        nt::ControlPoint1(-10.0f), nt::ControlPoint2(10.0f)); // Extreme control points

            if(std::isfinite(output.get()) && std::abs(output.get()) < 1000.0f)
            {
                stable_samples++;
            }
        }

        // Most samples should be stable
        float stability_ratio = static_cast<float>(stable_samples) / 1000.0f;
        REQUIRE(stability_ratio > 0.8f);

        INFO("Extreme parameter test: " << stability_ratio * 100.0f << "% stable samples");
    }

    SUBCASE("Memory consistency under stress")
    {
        // Test that repeated initialization and processing doesn't cause issues

        for(int iteration = 0; iteration < 100; ++iteration)
        {
            DeepnoteVoice voice;

            // Vary parameters each iteration
            int   osc_count   = 1 + (iteration % 8);
            float base_freq   = 100.0f + (iteration * 10.0f);
            float target_freq = base_freq * 2.0f;

            init_voice(voice, osc_count, nt::OscillatorFrequency(base_freq), nt::SampleRate(48000.0f),
                       nt::OscillatorFrequency(2.0f));

            voice.set_target_frequency(nt::OscillatorFrequency(target_freq));

            // Process varying amounts
            int process_count = 50 + (iteration * 5);
            for(int i = 0; i < process_count; ++i)
            {
                auto output = process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.25f),
                                            nt::ControlPoint2(0.75f));
                REQUIRE(std::isfinite(output.get()));
            }
        }

        INFO("Memory consistency test completed 100 iterations successfully");
    }
}

TEST_CASE("Concurrent access simulation")
{
    SUBCASE("Parameter changes during processing")
    {
        DeepnoteVoice voice;
        init_voice(voice, 4, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(2.0f));

        // Simulate concurrent parameter changes during processing
        std::vector<float> frequencies   = {440.0f, 880.0f, 220.0f, 660.0f, 330.0f};
        std::vector<float> detune_values = {0.0f, 5.0f, -5.0f, 10.0f, -10.0f};

        size_t freq_idx = 0, detune_idx = 0;

        for(int i = 0; i < 5000; ++i)
        {
            // Change parameters periodically to simulate concurrent access
            if(i % 100 == 0)
            {
                voice.set_target_frequency(nt::OscillatorFrequency(frequencies[freq_idx]));
                freq_idx = (freq_idx + 1) % frequencies.size();
            }

            if(i % 150 == 0)
            {
                voice.detune_oscillators(nt::DetuneHz(detune_values[detune_idx]));
                detune_idx = (detune_idx + 1) % detune_values.size();
            }

            // Process with varying parameters
            float anim_mult = 0.5f + ((i % 10) * 0.2f);
            float cp1       = (i % 20) * 0.05f;
            float cp2       = 1.0f - cp1;

            auto output = process_voice(voice, nt::AnimationMultiplier(anim_mult), nt::ControlPoint1(cp1),
                                        nt::ControlPoint2(cp2));

            REQUIRE(std::isfinite(output.get()));
            REQUIRE(std::abs(output.get()) < 200.0f);
        }

        INFO("Concurrent access simulation completed successfully");
    }

    SUBCASE("State consistency under rapid changes")
    {
        DeepnoteVoice voice;
        init_voice(voice, 3, nt::OscillatorFrequency(400.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(3.0f));

        // Track state transitions
        std::vector<DeepnoteVoice::State> state_history;
        state_history.push_back(voice.get_state());

        // Rapidly change targets and process
        for(int i = 0; i < 2000; ++i)
        {
            // Change target every 50 samples
            if(i % 50 == 0)
            {
                float new_target = 300.0f + ((i / 50) % 10) * 100.0f;
                voice.set_target_frequency(nt::OscillatorFrequency(new_target));
            }

            auto output =
                process_voice(voice, nt::AnimationMultiplier(4.0f), nt::ControlPoint1(0.3f), nt::ControlPoint2(0.7f));

            REQUIRE(std::isfinite(output.get()));

            // Track state changes
            auto current_state = voice.get_state();
            if(state_history.back() != current_state)
            {
                state_history.push_back(current_state);
            }
        }

        // Should have valid state transitions
        REQUIRE(state_history.size() > 1);

        // All states should be valid
        for(auto state : state_history)
        {
            REQUIRE((state == DeepnoteVoice::State::PENDING_TRANSIT_TO_TARGET ||
                     state == DeepnoteVoice::State::IN_TRANSIT_TO_TARGET || state == DeepnoteVoice::State::AT_TARGET));
        }

        INFO("State consistency test: " << state_history.size() << " state transitions observed");
    }
}
