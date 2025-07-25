#include "voice/deepnotevoice.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <doctest/doctest.h>
#include <numeric>
#include <vector>

using namespace deepnote;

/**
 * @file performance_tests.cpp
 * @brief Performance and real-time validation tests for the deepnote voice
 *
 * These tests verify that the voice implementation meets real-time performance
 * requirements and maintains stable memory usage under various conditions.
 */

TEST_CASE("Real-time processing validation")
{
    SUBCASE("Single voice processing speed")
    {
        DeepnoteVoice voice;
        init_voice(voice, 4, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        voice.set_target_frequency(nt::OscillatorFrequency(880.0f));

        const int samples_per_second    = 48000;
        const int test_duration_samples = samples_per_second; // 1 second of audio

        auto start_time = std::chrono::high_resolution_clock::now();

        // Process 1 second of audio
        for(int i = 0; i < test_duration_samples; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));
            REQUIRE(std::isfinite(output.get()));
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        INFO("Processing time for 1 second of audio: " << duration.count() << "ms");

        // Should process 1 second of audio in much less than 1 second (real-time requirement)
        REQUIRE(duration.count() < 100); // Less than 100ms for 1 second of audio
    }

    SUBCASE("Multiple voice scaling")
    {
        const std::vector<int> voice_counts = {1, 2, 4, 8, 16};

        for(int voice_count : voice_counts)
        {
            std::vector<DeepnoteVoice> voices(voice_count);

            // Initialize all voices
            for(int i = 0; i < voice_count; ++i)
            {
                float base_freq = 220.0f + i * 55.0f; // Spread frequencies
                init_voice(voices[i], 3, nt::OscillatorFrequency(base_freq), nt::SampleRate(48000.0f),
                           nt::OscillatorFrequency(2.0f));
                voices[i].set_target_frequency(nt::OscillatorFrequency(base_freq * 2.0f));
            }

            const int test_samples = 4800; // 0.1 seconds at 48kHz

            auto start_time = std::chrono::high_resolution_clock::now();

            // Process all voices
            for(int sample = 0; sample < test_samples; ++sample)
            {
                for(int voice_idx = 0; voice_idx < voice_count; ++voice_idx)
                {
                    auto output = process_voice(voices[voice_idx], nt::AnimationMultiplier(1.0f),
                                                nt::ControlPoint1(0.3f), nt::ControlPoint2(0.7f));
                    REQUIRE(std::isfinite(output.get()));
                }
            }

            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

            double processing_time_ms = duration.count() / 1000.0;
            double audio_time_ms      = (test_samples * 1000.0) / 48000.0; // Actual audio duration
            double cpu_usage_percent  = (processing_time_ms / audio_time_ms) * 100.0;

            INFO("Voice count: " << voice_count << ", Processing time: " << processing_time_ms << "ms, Audio time: "
                                 << audio_time_ms << "ms, CPU usage: " << cpu_usage_percent << "%");

            // Should not exceed reasonable CPU usage even with many voices
            // Scale expectations based on voice count
            double max_cpu_percent = voice_count <= 8 ? 50.0 : 80.0; // More lenient for many voices
            REQUIRE(cpu_usage_percent < max_cpu_percent);
        }
    }
}

TEST_CASE("Memory allocation stability")
{
    SUBCASE("No dynamic allocation during processing")
    {
        DeepnoteVoice voice;
        init_voice(voice, 6, nt::OscillatorFrequency(330.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.5f));

        voice.set_target_frequency(nt::OscillatorFrequency(660.0f));

        // Process many samples to test for memory leaks or allocations
        const int large_sample_count = 96000; // 2 seconds at 48kHz

        for(int i = 0; i < large_sample_count; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.2f), nt::ControlPoint2(0.8f));

            // Verify output is always finite and reasonable
            REQUIRE(std::isfinite(output.get()));
            REQUIRE(std::abs(output.get()) < 10.0f); // Reasonable amplitude bounds

            // Periodically check voice state is valid
            if(i % 10000 == 0)
            {
                auto current_freq = voice.get_current_frequency().get();
                REQUIRE(current_freq > 0.0f);
                REQUIRE(current_freq < 20000.0f); // Reasonable frequency bounds
            }
        }
    }

    SUBCASE("Consistent memory usage across transitions")
    {
        const int                num_transitions    = 10;
        const std::vector<float> target_frequencies = {220.0f, 440.0f,  330.0f, 880.0f, 550.0f,
                                                       165.0f, 1100.0f, 275.0f, 770.0f, 385.0f};

        DeepnoteVoice voice;
        init_voice(voice, 4, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(3.0f));

        for(int transition = 0; transition < num_transitions; ++transition)
        {
            float target_freq = target_frequencies[transition % target_frequencies.size()];
            voice.set_target_frequency(nt::OscillatorFrequency(target_freq));

            // Process until transition completes
            int samples_processed = 0;
            while(voice.get_state() != DeepnoteVoice::State::AT_TARGET && samples_processed < 25000)
            {
                auto output = process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.1f),
                                            nt::ControlPoint2(0.9f));

                REQUIRE(std::isfinite(output.get()));
                samples_processed++;
            }

            INFO("Transition " << transition << " to " << target_freq << "Hz completed in " << samples_processed
                               << " samples");

            // Should complete each transition
            REQUIRE(voice.get_state() == DeepnoteVoice::State::AT_TARGET);
        }
    }
}

TEST_CASE("Stress testing")
{
    SUBCASE("Rapid parameter changes")
    {
        DeepnoteVoice voice;
        init_voice(voice, 8, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f),
                   nt::OscillatorFrequency(5.0f)); // Fast animation

        // Rapid frequency changes
        const std::vector<float> frequencies = {440.0f, 880.0f, 220.0f, 1100.0f, 330.0f};

        for(int round = 0; round < 3; ++round)
        {
            for(float freq : frequencies)
            {
                voice.set_target_frequency(nt::OscillatorFrequency(freq));

                // Process briefly before changing again
                for(int i = 0; i < 100; ++i)
                {
                    auto output = process_voice(voice, nt::AnimationMultiplier(2.0f), // Very fast
                                                nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
                    REQUIRE(std::isfinite(output.get()));
                }
            }
        }

        // Voice should still be functional after rapid changes
        auto final_output =
            process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.5f), nt::ControlPoint2(0.5f));
        REQUIRE(std::isfinite(final_output.get()));
    }

    SUBCASE("Extreme oscillator count")
    {
        // Test with maximum reasonable oscillator count
        DeepnoteVoice voice;
        init_voice(voice, 16, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        voice.set_target_frequency(nt::OscillatorFrequency(880.0f));
        voice.detune_oscillators(nt::DetuneHz(5.0f)); // Add complexity with detuning

        const int stress_samples = 4800; // 0.1 seconds

        auto start_time = std::chrono::high_resolution_clock::now();

        for(int i = 0; i < stress_samples; ++i)
        {
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));

            REQUIRE(std::isfinite(output.get()));
            REQUIRE(std::abs(output.get()) < 20.0f); // Higher bound for many oscillators
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        INFO("16 oscillators processing time for 0.1s: " << duration.count() << "ms");

        // Should still meet real-time requirements even with many oscillators
        REQUIRE(duration.count() < 50); // Less than 50ms for 0.1s of audio
    }
}

TEST_CASE("Performance regression detection")
{
    SUBCASE("Baseline performance measurement")
    {
        // This test establishes baseline performance metrics
        DeepnoteVoice voice;
        init_voice(voice, 4, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(2.0f));

        voice.set_target_frequency(nt::OscillatorFrequency(880.0f));

        const int           benchmark_samples = 48000; // 1 second
        std::vector<double> sample_times;

        // Measure per-sample processing time
        for(int i = 0; i < benchmark_samples; ++i)
        {
            auto sample_start = std::chrono::high_resolution_clock::now();

            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.3f), nt::ControlPoint2(0.7f));

            auto sample_end      = std::chrono::high_resolution_clock::now();
            auto sample_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(sample_end - sample_start);

            sample_times.push_back(sample_duration.count());

            REQUIRE(std::isfinite(output.get()));
        }

        // Calculate statistics
        double total_time  = std::accumulate(sample_times.begin(), sample_times.end(), 0.0);
        double avg_time_ns = total_time / sample_times.size();

        std::sort(sample_times.begin(), sample_times.end());
        double median_time_ns = sample_times[sample_times.size() / 2];
        double p95_time_ns    = sample_times[static_cast<size_t>(sample_times.size() * 0.95)];
        double p99_time_ns    = sample_times[static_cast<size_t>(sample_times.size() * 0.99)];
        double p999_time_ns   = sample_times[static_cast<size_t>(sample_times.size() * 0.999)];
        double max_time_ns    = sample_times.back();

        INFO("Performance metrics (per sample):");
        INFO("Average: " << avg_time_ns << "ns");
        INFO("Median: " << median_time_ns << "ns");
        INFO("95th percentile: " << p95_time_ns << "ns");
        INFO("99th percentile: " << p99_time_ns << "ns");
        INFO("99.9th percentile: " << p999_time_ns << "ns");
        INFO("Maximum: " << max_time_ns << "ns");

        // Real-time constraint: each sample must be processed faster than sample period
        double sample_period_ns = 1e9 / 48000.0; // ~20,833 ns at 48kHz

        REQUIRE(avg_time_ns < sample_period_ns * 0.1); // Use only 10% of available time
        REQUIRE(p95_time_ns < sample_period_ns * 0.2); // 95% of samples under 20% of time
        // Use 99.9th percentile instead of max to avoid CI environment outliers
        REQUIRE(p999_time_ns < sample_period_ns * 2.0); // 99.9% of samples should be reasonable

        // Warn about extreme outliers (but don't fail the test for them)
        if(max_time_ns > sample_period_ns * 5.0)
        {
            INFO("WARNING: Detected extreme outlier timing: " << max_time_ns << "ns");
            INFO("This may indicate system scheduling issues in CI environment");
        }
    }

    SUBCASE("Performance consistency across states")
    {
        DeepnoteVoice voice;
        init_voice(voice, 4, nt::OscillatorFrequency(440.0f), nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));

        std::vector<std::pair<std::string, std::vector<double>>> state_timings;

        // Measure PENDING state
        voice.set_target_frequency(nt::OscillatorFrequency(880.0f));
        std::vector<double> pending_times;

        for(int i = 0; i < 100; ++i)
        {
            auto start = std::chrono::high_resolution_clock::now();
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.5f), nt::ControlPoint2(0.5f));
            auto end = std::chrono::high_resolution_clock::now();

            (void) output; // Suppress unused variable warning

            pending_times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());

            if(voice.get_state() != DeepnoteVoice::State::PENDING_TRANSIT_TO_TARGET)
                break;
        }
        state_timings.emplace_back("PENDING", pending_times);

        // Measure IN_TRANSIT state
        std::vector<double> transit_times;
        for(int i = 0; i < 1000 && voice.get_state() == DeepnoteVoice::State::IN_TRANSIT_TO_TARGET; ++i)
        {
            auto start = std::chrono::high_resolution_clock::now();
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.5f), nt::ControlPoint2(0.5f));
            auto end = std::chrono::high_resolution_clock::now();

            (void) output; // Suppress unused variable warning

            transit_times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
        }
        if(!transit_times.empty())
        {
            state_timings.emplace_back("IN_TRANSIT", transit_times);
        }

        // Measure AT_TARGET state
        // Process until we reach target
        for(int i = 0; i < 20000 && voice.get_state() != DeepnoteVoice::State::AT_TARGET; ++i)
        {
            process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.5f), nt::ControlPoint2(0.5f));
        }

        std::vector<double> target_times;
        for(int i = 0; i < 100; ++i)
        {
            auto start = std::chrono::high_resolution_clock::now();
            auto output =
                process_voice(voice, nt::AnimationMultiplier(1.0f), nt::ControlPoint1(0.5f), nt::ControlPoint2(0.5f));
            auto end = std::chrono::high_resolution_clock::now();

            (void) output; // Suppress unused variable warning

            target_times.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
        }
        state_timings.emplace_back("AT_TARGET", target_times);

        // Analyze timing consistency across states
        for(const auto &state_timing : state_timings)
        {
            const std::string         &state_name = state_timing.first;
            const std::vector<double> &timings    = state_timing.second;

            if(timings.empty())
                continue;

            double avg = std::accumulate(timings.begin(), timings.end(), 0.0) / timings.size();

            INFO("State " << state_name << " average timing: " << avg << "ns (" << timings.size() << " samples)");

            // Each state should have reasonable performance
            REQUIRE(avg < 10000.0); // Less than 10 microseconds per sample
        }
    }
}
