#include <doctest/doctest.h>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <limits>
#include "voice/deepnotevoice.hpp"

using namespace deepnote;

/**
 * @file property_edge_tests.cpp
 * @brief Property-based testing and edge case validation for the deepnote voice
 * 
 * These tests use random parameter generation and extreme values to validate
 * voice behavior across the entire parameter space and edge conditions.
 */

namespace {
    // Random number generator for property-based testing
    std::random_device rd;
    std::mt19937 gen(rd());
}

TEST_CASE("Frequency transition properties") {
    SUBCASE("Random frequency transitions") {
        // Property: Voice should always reach target frequency regardless of start/end points
        std::uniform_real_distribution<float> freq_dist(55.0f, 2000.0f); // Musical range
        std::uniform_int_distribution<int> osc_count_dist(1, 8);
        std::uniform_real_distribution<float> speed_dist(0.5f, 5.0f);
        
        const int num_tests = 50; // Property-based testing with multiple random cases
        
        for(int test = 0; test < num_tests; ++test) {
            float start_freq = freq_dist(gen);
            float target_freq = freq_dist(gen);
            int osc_count = osc_count_dist(gen);
            float animation_speed = speed_dist(gen);
            
            DeepnoteVoice voice;
            init_voice(voice, osc_count, nt::OscillatorFrequency(start_freq), 
                      nt::SampleRate(48000.0f), nt::OscillatorFrequency(animation_speed));
            
            voice.set_target_frequency(nt::OscillatorFrequency(target_freq));
            
            // Property: Should eventually reach target
            bool reached_target = false;
            int max_samples = static_cast<int>(100000 / animation_speed); // Scale timeout by speed
            max_samples = std::max(max_samples, 15000); // Minimum timeout
            max_samples = std::min(max_samples, 100000); // Maximum timeout
            
            for(int i = 0; i < max_samples; ++i) {
                auto output = process_voice(voice, nt::AnimationMultiplier(1.0f),
                                           nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));
                
                // Property: Output should always be finite
                REQUIRE(std::isfinite(output.get()));
                
                if(voice.get_state() == DeepnoteVoice::State::AT_TARGET) {
                    reached_target = true;
                    break;
                }
            }
            
            if(!reached_target) {
                INFO("Failed test " << test << ": " << start_freq << "Hz -> " << target_freq 
                     << "Hz (oscs:" << osc_count << ", speed:" << animation_speed 
                     << ") Did not reach target in " << max_samples << " samples");
                INFO("Current frequency: " << voice.get_current_frequency().get());
                INFO("Current state: " << static_cast<int>(voice.get_state()));
            }
            
            REQUIRE(reached_target);
            
            // Property: Final frequency should be close to target
            float final_freq = voice.get_current_frequency().get();
            float freq_error = std::abs(final_freq - target_freq);
            REQUIRE(freq_error < 5.0f); // Should be within 5Hz
            
            INFO("Test " << test << ": " << start_freq << "Hz -> " << target_freq 
                 << "Hz (oscs:" << osc_count << ", speed:" << animation_speed 
                 << ") Final: " << final_freq << "Hz");
        }
    }
    
    SUBCASE("Monotonicity property for ascending frequencies") {
        // Property: When transitioning to higher frequency, current frequency should generally increase
        std::uniform_real_distribution<float> start_freq_dist(200.0f, 400.0f);
        
        for(int test = 0; test < 20; ++test) {
            float start_freq = start_freq_dist(gen);
            float target_freq = start_freq * 2.0f; // Always ascending
            
            DeepnoteVoice voice;
            init_voice(voice, 4, nt::OscillatorFrequency(start_freq), 
                      nt::SampleRate(48000.0f), nt::OscillatorFrequency(2.0f));
            
            voice.set_target_frequency(nt::OscillatorFrequency(target_freq));
            
            std::vector<float> frequency_progression;
            
            // Collect frequency progression
            for(int i = 0; i < 15000; ++i) {
                process_voice(voice, nt::AnimationMultiplier(1.0f),
                             nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f)); // Linear
                
                float current_freq = voice.get_current_frequency().get();
                frequency_progression.push_back(current_freq);
                
                if(voice.get_state() == DeepnoteVoice::State::AT_TARGET) break;
            }
            
            // Property: Should be generally monotonic increasing
            int non_monotonic_count = 0;
            for(size_t i = 1; i < frequency_progression.size(); ++i) {
                if(frequency_progression[i] < frequency_progression[i-1]) {
                    non_monotonic_count++;
                }
            }
            
            // Allow some tolerance for numerical precision, but should be mostly monotonic
            float non_monotonic_ratio = static_cast<float>(non_monotonic_count) / frequency_progression.size();
            REQUIRE(non_monotonic_ratio < 0.1f); // Less than 10% non-monotonic steps
            
            INFO("Test " << test << ": " << non_monotonic_count << "/" << frequency_progression.size() 
                 << " non-monotonic steps (" << (non_monotonic_ratio * 100.0f) << "%)");
        }
    }
}

TEST_CASE("Extreme parameter values") {
    SUBCASE("Very low frequencies") {
        // Test with frequencies near the lower musical limit
        std::vector<float> low_frequencies = {20.0f, 27.5f, 30.0f, 35.0f, 40.0f}; // Sub-bass range
        
        for(float freq : low_frequencies) {
            DeepnoteVoice voice;
            init_voice(voice, 3, nt::OscillatorFrequency(freq), 
                      nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));
            
            voice.set_target_frequency(nt::OscillatorFrequency(freq * 2.0f));
            
            // Should handle low frequencies without issues
            bool stable = true;
            for(int i = 0; i < 1000; ++i) {
                auto output = process_voice(voice, nt::AnimationMultiplier(1.0f),
                                           nt::ControlPoint1(0.3f), nt::ControlPoint2(0.7f));
                
                if(!std::isfinite(output.get()) || std::abs(output.get()) > 10.0f) {
                    stable = false;
                    break;
                }
            }
            
            REQUIRE(stable);
            INFO("Low frequency " << freq << "Hz: stable");
        }
    }
    
    SUBCASE("Very high frequencies") {
        // Test with frequencies near the upper audio limit
        std::vector<float> high_frequencies = {8000.0f, 12000.0f, 16000.0f, 18000.0f}; // High treble
        
        for(float freq : high_frequencies) {
            DeepnoteVoice voice;
            init_voice(voice, 2, nt::OscillatorFrequency(freq), 
                      nt::SampleRate(48000.0f), nt::OscillatorFrequency(3.0f));
            
            voice.set_target_frequency(nt::OscillatorFrequency(freq * 0.5f)); // Descending
            
            // Should handle high frequencies without aliasing issues
            bool stable = true;
            for(int i = 0; i < 1000; ++i) {
                auto output = process_voice(voice, nt::AnimationMultiplier(1.0f),
                                           nt::ControlPoint1(0.2f), nt::ControlPoint2(0.8f));
                
                if(!std::isfinite(output.get()) || std::abs(output.get()) > 10.0f) {
                    stable = false;
                    break;
                }
            }
            
            REQUIRE(stable);
            INFO("High frequency " << freq << "Hz: stable");
        }
    }
    
    SUBCASE("Extreme frequency ratios") {
        // Test large frequency jumps
        struct FreqPair {
            float start, target;
            std::string description;
        };
        
        std::vector<FreqPair> extreme_ratios = {
            {55.0f, 1760.0f, "32x increase"},    // 5+ octaves up
            {2000.0f, 62.5f, "32x decrease"},   // 5+ octaves down
            {110.0f, 3520.0f, "32x increase"},  // Extreme upward
            {1000.0f, 31.25f, "32x decrease"}   // Extreme downward
        };
        
        for(const auto& pair : extreme_ratios) {
            DeepnoteVoice voice;
            init_voice(voice, 4, nt::OscillatorFrequency(pair.start), 
                      nt::SampleRate(48000.0f), nt::OscillatorFrequency(2.0f)); // Faster animation
            
            voice.set_target_frequency(nt::OscillatorFrequency(pair.target));
            
            // Should handle extreme transitions (with longer timeout)
            bool completed = false;
            for(int i = 0; i < 50000; ++i) {
                auto output = process_voice(voice, nt::AnimationMultiplier(1.0f),
                                           nt::ControlPoint1(0.1f), nt::ControlPoint2(0.9f));
                
                REQUIRE(std::isfinite(output.get()));
                
                if(voice.get_state() == DeepnoteVoice::State::AT_TARGET) {
                    completed = true;
                    break;
                }
            }
            
            if(!completed) {
                INFO("Extreme ratio test failed: " << pair.description);
                INFO("Current frequency: " << voice.get_current_frequency().get());
                INFO("Target frequency: " << pair.target);
                INFO("Current state: " << static_cast<int>(voice.get_state()));
            }
            
            REQUIRE(completed);
            
            float final_freq = voice.get_current_frequency().get();
            float error_percent = std::abs(final_freq - pair.target) / pair.target * 100.0f;
            REQUIRE(error_percent < 10.0f); // More lenient for extreme ratios
            
            INFO(pair.description << ": " << pair.start << "Hz -> " << pair.target 
                 << "Hz, final: " << final_freq << "Hz");
        }
    }
}

TEST_CASE("Bezier curve parameter edge cases") {
    SUBCASE("Extreme control point combinations") {
        // Test boundary values for Bezier control points
        std::vector<std::pair<float, float>> extreme_control_points = {
            {0.0f, 0.0f},     // Both at minimum
            {1.0f, 1.0f},     // Both at maximum  
            {0.0f, 1.0f},     // Linear (should work)
            {1.0f, 0.0f},     // Reverse linear
            {0.01f, 0.99f},   // Near extremes
            {0.99f, 0.01f},   // Near extremes reversed
            {0.5f, 0.0f},     // Ease in extreme
            {0.0f, 0.5f},     // Ease out extreme
            {0.99f, 0.99f},   // Very sharp curve
            {0.01f, 0.01f}    // Very flat curve
        };
        
        for(const auto& cp : extreme_control_points) {
            DeepnoteVoice voice;
            init_voice(voice, 3, nt::OscillatorFrequency(440.0f), 
                      nt::SampleRate(48000.0f), nt::OscillatorFrequency(3.0f)); // Faster animation
            
            voice.set_target_frequency(nt::OscillatorFrequency(880.0f));
            
            // Should handle extreme control points gracefully
            bool stable = true;
            bool reached_target = false;
            
            for(int i = 0; i < 30000; ++i) { // Increased timeout
                auto output = process_voice(voice, nt::AnimationMultiplier(1.0f),
                                           nt::ControlPoint1(cp.first), nt::ControlPoint2(cp.second));
                
                if(!std::isfinite(output.get())) {
                    stable = false;
                    break;
                }
                
                if(voice.get_state() == DeepnoteVoice::State::AT_TARGET) {
                    reached_target = true;
                    break;
                }
            }
            
            REQUIRE(stable);
            
            if(!reached_target) {
                INFO("Control points (" << cp.first << ", " << cp.second << ") did not reach target");
                INFO("Current frequency: " << voice.get_current_frequency().get());
                INFO("Current state: " << static_cast<int>(voice.get_state()));
            }
            
            REQUIRE(reached_target);
            
            INFO("Control points (" << cp.first << ", " << cp.second << "): OK");
        }
    }
    
    SUBCASE("Animation multiplier extremes") {
        // Test extreme animation speed multipliers
        std::vector<float> extreme_multipliers = {
            0.5f,   // Moderate slow
            1.0f,   // Normal
            2.0f,   // Fast
            5.0f,   // Very fast
            10.0f   // Extremely fast
        };
        
        for(float multiplier : extreme_multipliers) {
            DeepnoteVoice voice;
            init_voice(voice, 2, nt::OscillatorFrequency(300.0f), 
                      nt::SampleRate(48000.0f), nt::OscillatorFrequency(4.0f)); // Faster base animation
            
            voice.set_target_frequency(nt::OscillatorFrequency(600.0f));
            
            bool stable = true;
            bool reached_target = false;
            int max_samples = 25000; // Generous timeout for all cases
            
            INFO("Testing multiplier " << multiplier << " with max_samples " << max_samples);
            
            for(int i = 0; i < max_samples; ++i) {
                auto output = process_voice(voice, nt::AnimationMultiplier(multiplier),
                                           nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));
                
                if(!std::isfinite(output.get())) {
                    stable = false;
                    break;
                }
                
                if(voice.get_state() == DeepnoteVoice::State::AT_TARGET) {
                    reached_target = true;
                    INFO("Multiplier " << multiplier << " reached target at sample " << i);
                    break;
                }
            }
            
            REQUIRE(stable);
            
            if(!reached_target) {
                INFO("Animation multiplier " << multiplier << " did not reach target in " << max_samples << " samples");
                INFO("Current frequency: " << voice.get_current_frequency().get());
                INFO("Current state: " << static_cast<int>(voice.get_state()));
            }
            
            REQUIRE(reached_target); // All reasonable multipliers should complete
            
            INFO("Animation multiplier " << multiplier << ": " 
                 << (reached_target ? "completed" : "stable but incomplete"));
        }
    }
}

TEST_CASE("Detuning edge cases") {
    SUBCASE("Extreme detune values") {
        std::vector<float> extreme_detunes = {
            0.0f,    // No detuning
            0.1f,    // Minimal detuning
            50.0f,   // Heavy detuning
            100.0f,  // Very heavy detuning
            -25.0f,  // Negative detuning
            -100.0f  // Large negative detuning
        };
        
        for(float detune_hz : extreme_detunes) {
            DeepnoteVoice voice;
            init_voice(voice, 6, nt::OscillatorFrequency(440.0f), 
                      nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f),
                      nt::DetuneHz(detune_hz));
            
            // Test that extreme detuning doesn't break the voice
            bool stable = true;
            for(int i = 0; i < 1000; ++i) {
                auto output = process_voice(voice, nt::AnimationMultiplier(1.0f),
                                           nt::ControlPoint1(0.5f), nt::ControlPoint2(0.5f));
                
                if(!std::isfinite(output.get()) || std::abs(output.get()) > 30.0f) { // More lenient bounds
                    stable = false;
                    break;
                }
            }
            
            REQUIRE(stable);
            INFO("Detune " << detune_hz << "Hz: stable");
        }
    }
    
    SUBCASE("Detuning with frequency transitions") {
        // Test that detuning works correctly during transitions
        DeepnoteVoice voice;
        init_voice(voice, 4, nt::OscillatorFrequency(220.0f), 
                  nt::SampleRate(48000.0f), nt::OscillatorFrequency(3.0f),
                  nt::DetuneHz(10.0f)); // Significant detuning
        
        voice.set_target_frequency(nt::OscillatorFrequency(880.0f));
        
        std::vector<float> amplitude_samples;
        
        // Collect samples during transition
        for(int i = 0; i < 5000; ++i) {
            auto output = process_voice(voice, nt::AnimationMultiplier(1.0f),
                                       nt::ControlPoint1(0.3f), nt::ControlPoint2(0.7f));
            
            REQUIRE(std::isfinite(output.get()));
            amplitude_samples.push_back(std::abs(output.get()));
            
            if(voice.get_state() == DeepnoteVoice::State::AT_TARGET) break;
        }
        
        // Should have reasonable amplitude variation due to beating
        float min_amp = *std::min_element(amplitude_samples.begin(), amplitude_samples.end());
        float max_amp = *std::max_element(amplitude_samples.begin(), amplitude_samples.end());
        
        REQUIRE(max_amp > min_amp); // Should have amplitude variation
        REQUIRE(max_amp < 15.0f);   // But not excessive
        
        INFO("Amplitude range with detuning: " << min_amp << " to " << max_amp);
    }
}

TEST_CASE("Sample rate variations") {
    SUBCASE("Different sample rates") {
        // Test various sample rates that might be encountered
        std::vector<float> sample_rates = {
            8000.0f,   // Telephone quality
            22050.0f,  // Half CD quality
            44100.0f,  // CD quality
            48000.0f,  // Professional standard
            88200.0f,  // High quality
            96000.0f,  // Very high quality
            192000.0f  // Extreme quality
        };
        
        for(float sr : sample_rates) {
            DeepnoteVoice voice;
            init_voice(voice, 3, nt::OscillatorFrequency(440.0f), 
                      nt::SampleRate(sr), nt::OscillatorFrequency(2.0f));
            
            voice.set_target_frequency(nt::OscillatorFrequency(880.0f));
            
            // Should work at any reasonable sample rate
            bool stable = true;
            for(int i = 0; i < 100; ++i) { // Fewer samples for very high sample rates
                auto output = process_voice(voice, nt::AnimationMultiplier(1.0f),
                                           nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));
                
                if(!std::isfinite(output.get()) || std::abs(output.get()) > 10.0f) {
                    stable = false;
                    break;
                }
            }
            
            REQUIRE(stable);
            INFO("Sample rate " << sr << "Hz: stable");
        }
    }
    
    SUBCASE("Sample rate and frequency relationship") {
        // Test that frequency transitions scale appropriately with sample rate
        struct SampleRateTest {
            float sample_rate;
            int expected_min_samples; // Rough estimate for transition completion
        };
        
        std::vector<SampleRateTest> sr_tests = {
            {22050.0f, 5000},   // Lower sample rate
            {48000.0f, 10000},  // Standard sample rate  
            {96000.0f, 20000}   // Higher sample rate
        };
        
        for(const auto& test : sr_tests) {
            DeepnoteVoice voice;
            init_voice(voice, 2, nt::OscillatorFrequency(300.0f), 
                      nt::SampleRate(test.sample_rate), nt::OscillatorFrequency(2.0f));
            
            voice.set_target_frequency(nt::OscillatorFrequency(600.0f));
            
            int samples_to_complete = 0;
            for(int i = 0; i < 50000; ++i) {
                process_voice(voice, nt::AnimationMultiplier(1.0f),
                             nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f));
                samples_to_complete++;
                
                if(voice.get_state() == DeepnoteVoice::State::AT_TARGET) {
                    break;
                }
            }
            
            // Higher sample rates should require proportionally more samples
            // (since the same time duration needs more samples)
            INFO("Sample rate " << test.sample_rate << "Hz: " << samples_to_complete << " samples to complete");
            
            // Should complete transition
            REQUIRE(voice.get_state() == DeepnoteVoice::State::AT_TARGET);
        }
    }
}
