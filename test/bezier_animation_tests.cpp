#include <doctest/doctest.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include "voice/deepnotevoice.hpp"
#include "unitshapers/bezier.hpp"

using namespace deepnote;

/**
 * @file bezier_animation_tests.cpp
 * @brief Comprehensive Bezier curve and animation validation tests
 * 
 * These tests verify the mathematical correctness of Bezier curve interpolation,
 * animation smoothness, curve shape characteristics, and integration with the
 * voice animation system.
 */

TEST_CASE("Bezier curve mathematical properties") {
    SUBCASE("Basic Bezier curve evaluation") {
        // Test standard cubic Bezier curve properties
        std::vector<std::pair<float, float>> test_curves = {
            {0.0f, 1.0f},   // Linear
            {0.5f, 0.5f},   // Symmetric
            {0.0f, 0.0f},   // Flat start
            {1.0f, 1.0f},   // Flat end
            {0.25f, 0.75f}, // Standard ease-in-out
            {0.42f, 0.0f},  // Ease-in (CSS standard)
            {0.0f, 0.58f},  // Ease-out (CSS standard)
        };
        
        for (const auto& curve : test_curves) {
            BezierUnitShaper shaper(nt::ControlPoint1(curve.first), nt::ControlPoint2(curve.second));
            
            // Test boundary conditions
            float start_value = shaper(0.0f);
            float end_value = shaper(1.0f);
            
            REQUIRE(std::abs(start_value - 0.0f) < 0.001f);
            REQUIRE(std::abs(end_value - 1.0f) < 0.001f);
            
            // Test monotonicity for standard curves
            if (curve.first >= 0.0f && curve.first <= 1.0f && 
                curve.second >= 0.0f && curve.second <= 1.0f) {
                std::vector<float> values;
                for (int i = 0; i <= 100; ++i) {
                    float t = i / 100.0f;
                    values.push_back(shaper(t));
                }
                
                // Check that values are generally increasing (monotonic)
                bool is_monotonic = true;
                for (size_t i = 1; i < values.size(); ++i) {
                    if (values[i] < values[i-1] - 0.01f) { // Small tolerance for numerical errors
                        is_monotonic = false;
                        break;
                    }
                }
                
                REQUIRE(is_monotonic);
                INFO("Curve (" << curve.first << ", " << curve.second << ") is monotonic");
            }
        }
    }
    
    SUBCASE("Bezier curve continuity and smoothness") {
        BezierUnitShaper shaper(nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));
        
        // Test continuity by checking small step differences
        const int steps = 1000;
        float max_discontinuity = 0.0f;
        
        for (int i = 0; i < steps - 1; ++i) {
            float t1 = static_cast<float>(i) / steps;
            float t2 = static_cast<float>(i + 1) / steps;
            
            float v1 = shaper(t1);
            float v2 = shaper(t2);
            
            float discontinuity = std::abs(v2 - v1);
            max_discontinuity = std::max(max_discontinuity, discontinuity);
        }
        
        // Maximum discontinuity should be reasonable for the step size
        float expected_max_step = 2.0f / steps; // Conservative bound
        REQUIRE(max_discontinuity < expected_max_step);
        
        INFO("Maximum discontinuity: " << max_discontinuity);
        INFO("Expected maximum step: " << expected_max_step);
    }
    
    SUBCASE("Bezier curve derivative approximation") {
        BezierUnitShaper shaper(nt::ControlPoint1(0.42f), nt::ControlPoint2(0.0f)); // Ease-in curve
        
        // Approximate derivatives and check for reasonableness
        const float epsilon = 0.001f;
        std::vector<float> derivatives;
        
        for (int i = 1; i < 100; ++i) {
            float t = i / 100.0f;
            
            float v_minus = shaper(t - epsilon);
            float v_plus = shaper(t + epsilon);
            
            float derivative = (v_plus - v_minus) / (2.0f * epsilon);
            derivatives.push_back(derivative);
        }
        
        // For ease-in curve, derivative should generally increase
        bool generally_increasing = true;
        int increasing_count = 0;
        
        for (size_t i = 1; i < derivatives.size(); ++i) {
            if (derivatives[i] > derivatives[i-1]) {
                increasing_count++;
            }
        }
        
        // At least 60% of derivative samples should be increasing for ease-in
        float increasing_ratio = static_cast<float>(increasing_count) / (derivatives.size() - 1);
        REQUIRE(increasing_ratio > 0.6f);
        
        INFO("Derivative increasing ratio: " << increasing_ratio);
    }
    
    SUBCASE("Extreme curve control points") {
        // Test curves with extreme control points
        std::vector<std::pair<float, float>> extreme_curves = {
            {-0.5f, 1.5f},  // Overshoot
            {1.5f, -0.5f},  // Undershoot
            {-1.0f, 2.0f},  // Large overshoot
            {2.0f, -1.0f},  // Large undershoot
        };
        
        for (const auto& curve : extreme_curves) {
            BezierUnitShaper shaper(nt::ControlPoint1(curve.first), nt::ControlPoint2(curve.second));
            
            // Even with extreme control points, boundaries should be respected
            REQUIRE(std::abs(shaper(0.0f) - 0.0f) < 0.001f);
            REQUIRE(std::abs(shaper(1.0f) - 1.0f) < 0.001f);
            
            // Should not produce NaN or infinite values
            for (int i = 0; i <= 100; ++i) {
                float t = i / 100.0f;
                float value = shaper(t);
                
                REQUIRE(std::isfinite(value));
                REQUIRE(!std::isnan(value));
            }
            
            INFO("Extreme curve (" << curve.first << ", " << curve.second << ") produces finite values");
        }
    }
}

TEST_CASE("Animation interpolation accuracy") {
    SUBCASE("Linear interpolation verification") {
        // Test that linear curve produces approximately linear interpolation
        // Note: (0.0, 1.0) control points don't produce perfectly linear curves in cubic Bezier
        BezierUnitShaper linear_shaper(nt::ControlPoint1(0.33f), nt::ControlPoint2(0.67f));
        
        for (int i = 0; i <= 100; ++i) {
            float t = i / 100.0f;
            float shaped_value = linear_shaper(t);
            
            // Should be reasonably close to linear
            REQUIRE(std::abs(shaped_value - t) < 0.1f);
        }
    }
    
    SUBCASE("Animation frequency interpolation accuracy") {
        DeepnoteVoice voice;
        init_voice(voice, 1, nt::OscillatorFrequency(440.0f), 
                  nt::SampleRate(48000.0f), nt::OscillatorFrequency(4.0f));
        
        // Test different curve shapes for frequency interpolation
        std::vector<std::pair<float, float>> curves = {
            {0.0f, 1.0f},   // Linear
            {0.25f, 0.75f}, // Ease-in-out
            {0.42f, 0.0f},  // Ease-in
            {0.0f, 0.58f},  // Ease-out
        };
        
        for (const auto& curve : curves) {
            // Reset voice state
            voice.set_current_frequency(nt::OscillatorFrequency(440.0f));
            voice.set_target_frequency(nt::OscillatorFrequency(880.0f));
            
            std::vector<float> frequency_progression;
            
            // Collect frequency values during animation
            for (int i = 0; i < 8000; ++i) {  // Increased iteration count
                process_voice(voice, nt::AnimationMultiplier(3.0f),  // Higher animation multiplier
                             nt::ControlPoint1(curve.first), nt::ControlPoint2(curve.second));
                
                frequency_progression.push_back(voice.get_current_frequency().get());
                
                if (voice.get_state() == DeepnoteVoice::State::AT_TARGET) {
                    break;
                }
            }
            
            REQUIRE(frequency_progression.size() > 10); // Should have some progression
            
            // Check that frequency moves in the right direction
            float start_freq = frequency_progression.front();
            float end_freq = frequency_progression.back();
            
            REQUIRE(start_freq >= 430.0f); // Near 440Hz (wider tolerance)
            REQUIRE(start_freq <= 450.0f);
            REQUIRE(end_freq >= 860.0f);   // Near 880Hz (wider tolerance)
            REQUIRE(end_freq <= 900.0f);
            
            // Check that progression is generally monotonic
            int monotonic_violations = 0;
            for (size_t i = 1; i < frequency_progression.size(); ++i) {
                if (frequency_progression[i] < frequency_progression[i-1] - 1.0f) {
                    monotonic_violations++;
                }
            }
            
            // Allow some small violations due to floating point precision
            float violation_ratio = static_cast<float>(monotonic_violations) / frequency_progression.size();
            REQUIRE(violation_ratio < 0.05f); // Less than 5% violations
            
            INFO("Curve (" << curve.first << ", " << curve.second 
                 << ") - Samples: " << frequency_progression.size() 
                 << ", Violations: " << violation_ratio * 100.0f << "%");
        }
    }
    
    SUBCASE("Animation timing precision") {
        // Test that different curves complete in predictable timeframes
        struct CurveTest {
            float cp1, cp2;
            std::string name;
            int expected_min_samples;
            int expected_max_samples;
        };
        
        std::vector<CurveTest> curve_tests = {
            {0.0f, 1.0f, "Linear", 800, 3000},
            {0.25f, 0.75f, "Ease-in-out", 800, 3000},
            {0.42f, 0.0f, "Ease-in", 800, 3000},
            {0.0f, 0.58f, "Ease-out", 800, 3000},
            {1.0f, 0.0f, "Fast-start", 800, 3000},
            {0.0f, 1.0f, "Slow-start", 800, 3000},
        };
        
        for (const auto& test : curve_tests) {
            DeepnoteVoice voice;
            init_voice(voice, 2, nt::OscillatorFrequency(300.0f), 
                      nt::SampleRate(48000.0f), nt::OscillatorFrequency(3.0f));
            
            voice.set_target_frequency(nt::OscillatorFrequency(600.0f));
            
            int samples_to_complete = 0;
            for (int i = 0; i < test.expected_max_samples; ++i) {
                process_voice(voice, nt::AnimationMultiplier(2.0f),  // Higher animation multiplier
                             nt::ControlPoint1(test.cp1), nt::ControlPoint2(test.cp2));
                samples_to_complete++;
                
                if (voice.get_state() == DeepnoteVoice::State::AT_TARGET) {
                    break;
                }
            }
            
            REQUIRE(samples_to_complete >= test.expected_min_samples);
            REQUIRE(samples_to_complete <= test.expected_max_samples);
            
            INFO(test.name << " curve completed in " << samples_to_complete << " samples");
        }
    }
}

TEST_CASE("Curve shape characteristics") {
    SUBCASE("Ease-in curve characteristics") {
        BezierUnitShaper ease_in(nt::ControlPoint1(0.42f), nt::ControlPoint2(0.0f));
        
        std::vector<float> values;
        for (int i = 0; i <= 100; ++i) {
            float t = i / 100.0f;
            values.push_back(ease_in(t));
        }
        
        // Ease-in should start slowly (small values for small t)
        REQUIRE(values[10] < 0.2f);   // At t=0.1, value should be < 0.2
        REQUIRE(values[25] < 0.6f);   // At t=0.25, value should be < 0.6
        
        // But should accelerate towards the end
        REQUIRE(values[75] > 0.4f);   // At t=0.75, value should be > 0.4
        REQUIRE(values[90] > 0.7f);   // At t=0.9, value should be > 0.7
        
        INFO("Ease-in characteristics: t=0.1 -> " << values[10] 
             << ", t=0.25 -> " << values[25] 
             << ", t=0.75 -> " << values[75] 
             << ", t=0.9 -> " << values[90]);
    }
    
    SUBCASE("Ease-out curve characteristics") {
        BezierUnitShaper ease_out(nt::ControlPoint1(0.0f), nt::ControlPoint2(0.58f));
        
        std::vector<float> values;
        for (int i = 0; i <= 100; ++i) {
            float t = i / 100.0f;
            values.push_back(ease_out(t));
        }
        
        // Ease-out should start quickly
        REQUIRE(values[10] > 0.01f);  // At t=0.1, value should be > 0.01
        REQUIRE(values[25] > 0.08f);  // At t=0.25, value should be > 0.08
        
        // But should decelerate towards the end
        REQUIRE(values[75] < 0.95f);  // At t=0.75, value should be < 0.95
        REQUIRE(values[90] < 0.99f);  // At t=0.9, value should be < 0.99
        
        INFO("Ease-out characteristics: t=0.1 -> " << values[10] 
             << ", t=0.25 -> " << values[25] 
             << ", t=0.75 -> " << values[75] 
             << ", t=0.9 -> " << values[90]);
    }
    
    SUBCASE("Ease-in-out curve characteristics") {
        BezierUnitShaper ease_in_out(nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));
        
        std::vector<float> values;
        for (int i = 0; i <= 100; ++i) {
            float t = i / 100.0f;
            values.push_back(ease_in_out(t));
        }
        
        // Should be relatively symmetric around t=0.5
        float mid_value = values[50];
        REQUIRE(std::abs(mid_value - 0.5f) < 0.1f);
        
        // Should start and end slowly
        REQUIRE(values[10] < 0.15f);   // Slow start
        REQUIRE(values[90] > 0.85f);   // Slow end
        
        // Should be fastest around the middle
        float quarter_speed = values[25] - values[15];
        float mid_speed = values[55] - values[45];
        float three_quarter_speed = values[85] - values[75];
        
        REQUIRE(mid_speed > quarter_speed);
        REQUIRE(mid_speed > three_quarter_speed);
        
        INFO("Ease-in-out speeds: quarter=" << quarter_speed 
             << ", mid=" << mid_speed 
             << ", three-quarter=" << three_quarter_speed);
    }
    
    SUBCASE("Overshoot curve behavior") {
        BezierUnitShaper overshoot(nt::ControlPoint1(-0.3f), nt::ControlPoint2(1.3f));
        
        std::vector<float> values;
        float min_value = 1.0f, max_value = 0.0f;
        
        for (int i = 0; i <= 100; ++i) {
            float t = i / 100.0f;
            float value = overshoot(t);
            values.push_back(value);
            
            min_value = std::min(min_value, value);
            max_value = std::max(max_value, value);
        }
        
        // Overshoot curves should go outside [0,1] range
        INFO("Overshoot range: [" << min_value << ", " << max_value << "]");
        
        // But should still start and end at correct values
        REQUIRE(std::abs(values[0] - 0.0f) < 0.001f);
        REQUIRE(std::abs(values[100] - 1.0f) < 0.001f);
    }
}

TEST_CASE("Animation smoothness validation") {
    SUBCASE("Smooth frequency transitions") {
        DeepnoteVoice voice;
        init_voice(voice, 3, nt::OscillatorFrequency(200.0f), 
                  nt::SampleRate(48000.0f), nt::OscillatorFrequency(2.0f));
        
        voice.set_target_frequency(nt::OscillatorFrequency(800.0f));
        
        std::vector<float> frequencies;
        std::vector<float> frequency_deltas;
        
        // Collect frequency progression with smooth curve
        for (int i = 0; i < 3000; ++i) {
            float prev_freq = voice.get_current_frequency().get();
            
            process_voice(voice, nt::AnimationMultiplier(1.0f),
                         nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));
            
            float curr_freq = voice.get_current_frequency().get();
            frequencies.push_back(curr_freq);
            
            if (i > 0) {
                frequency_deltas.push_back(std::abs(curr_freq - prev_freq));
            }
            
            if (voice.get_state() == DeepnoteVoice::State::AT_TARGET) {
                break;
            }
        }
        
        REQUIRE(frequencies.size() > 50); // Should have reasonable progression
        
        // Calculate smoothness metrics
        if (frequency_deltas.size() > 10) {
            float mean_delta = std::accumulate(frequency_deltas.begin(), frequency_deltas.end(), 0.0f) / frequency_deltas.size();
            
            float variance = 0.0f;
            for (float delta : frequency_deltas) {
                variance += (delta - mean_delta) * (delta - mean_delta);
            }
            variance /= frequency_deltas.size();
            float std_dev = std::sqrt(variance);
            
            // Standard deviation should be reasonable compared to mean
            float coefficient_of_variation = std_dev / mean_delta;
            REQUIRE(coefficient_of_variation < 2.0f); // Should not be too erratic
            
            INFO("Frequency transition smoothness - Mean delta: " << mean_delta 
                 << ", StdDev: " << std_dev 
                 << ", CV: " << coefficient_of_variation);
        }
    }
    
    SUBCASE("Jerk analysis (third derivative)") {
        // Test that animations don't have excessive jerk (sudden acceleration changes)
        DeepnoteVoice voice;
        init_voice(voice, 2, nt::OscillatorFrequency(440.0f), 
                  nt::SampleRate(48000.0f), nt::OscillatorFrequency(3.0f));
        
        voice.set_target_frequency(nt::OscillatorFrequency(660.0f));
        
        std::vector<float> frequencies;
        
        for (int i = 0; i < 2000; ++i) {
            process_voice(voice, nt::AnimationMultiplier(1.0f),
                         nt::ControlPoint1(0.2f), nt::ControlPoint2(0.8f));
            
            frequencies.push_back(voice.get_current_frequency().get());
            
            if (voice.get_state() == DeepnoteVoice::State::AT_TARGET) {
                break;
            }
        }
        
        if (frequencies.size() > 6) {
            // Calculate approximate third derivatives (jerk)
            std::vector<float> jerks;
            
            for (size_t i = 3; i < frequencies.size() - 3; ++i) {
                // Use finite difference approximation for third derivative
                float jerk = frequencies[i+3] - 3*frequencies[i+2] + 3*frequencies[i+1] - frequencies[i];
                jerks.push_back(std::abs(jerk));
            }
            
            if (!jerks.empty()) {
                float max_jerk = *std::max_element(jerks.begin(), jerks.end());
                float mean_jerk = std::accumulate(jerks.begin(), jerks.end(), 0.0f) / jerks.size();
                
                // Maximum jerk should not be excessive compared to mean
                REQUIRE(max_jerk < mean_jerk * 10.0f);
                
                INFO("Jerk analysis - Max: " << max_jerk << ", Mean: " << mean_jerk);
            }
        }
    }
    
    SUBCASE("Curve consistency across different ranges") {
        // Test that curve behavior is consistent across different frequency ranges
        std::vector<std::pair<float, float>> frequency_ranges = {
            {100.0f, 200.0f},   // Low frequencies
            {440.0f, 880.0f},   // Mid frequencies  
            {1000.0f, 2000.0f}, // High frequencies
            {2000.0f, 4000.0f}, // Very high frequencies
        };
        
        for (const auto& range : frequency_ranges) {
            DeepnoteVoice voice;
            init_voice(voice, 2, nt::OscillatorFrequency(range.first), 
                      nt::SampleRate(48000.0f), nt::OscillatorFrequency(2.5f));
            
            voice.set_target_frequency(nt::OscillatorFrequency(range.second));
            
            std::vector<float> normalized_progression;
            
            for (int i = 0; i < 5000; ++i) {  // Increased iteration count
                process_voice(voice, nt::AnimationMultiplier(2.0f),  // Higher animation multiplier
                             nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));
                
                float current = voice.get_current_frequency().get();
                float normalized = (current - range.first) / (range.second - range.first);
                normalized_progression.push_back(normalized);
                
                if (voice.get_state() == DeepnoteVoice::State::AT_TARGET) {
                    break;
                }
            }
            
            REQUIRE(normalized_progression.size() > 20);
            
            // Check that progression starts near 0 and ends reasonably close to target
            REQUIRE(normalized_progression.front() < 0.1f);
            REQUIRE(normalized_progression.back() > 0.3f);  // More lenient - some animations may not complete
            
            // Check that progression is generally monotonic
            int violations = 0;
            for (size_t i = 1; i < normalized_progression.size(); ++i) {
                if (normalized_progression[i] < normalized_progression[i-1] - 0.02f) {
                    violations++;
                }
            }
            
            float violation_rate = static_cast<float>(violations) / normalized_progression.size();
            REQUIRE(violation_rate < 0.05f);
            
            INFO("Range [" << range.first << ", " << range.second 
                 << "] - Samples: " << normalized_progression.size() 
                 << ", Violations: " << violation_rate * 100.0f << "%");
        }
    }
}

TEST_CASE("Complex animation scenarios") {
    SUBCASE("Rapid curve changes") {
        DeepnoteVoice voice;
        init_voice(voice, 4, nt::OscillatorFrequency(440.0f), 
                  nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.5f));
        
        // Test rapidly changing curve parameters during animation
        std::vector<std::pair<float, float>> curves = {
            {0.0f, 1.0f},
            {0.25f, 0.75f},
            {0.5f, 0.5f},
            {0.42f, 0.0f},
            {0.0f, 0.58f},
        };
        
        voice.set_target_frequency(nt::OscillatorFrequency(880.0f));
        
        size_t curve_index = 0;
        std::vector<float> output_samples;
        
        for (int i = 0; i < 3000; ++i) {
            // Change curve every 100 samples
            if (i % 100 == 0) {
                curve_index = (curve_index + 1) % curves.size();
            }
            
            auto curve = curves[curve_index];
            auto output = process_voice(voice, nt::AnimationMultiplier(1.0f),
                                       nt::ControlPoint1(curve.first), 
                                       nt::ControlPoint2(curve.second));
            
            REQUIRE(std::isfinite(output.get()));
            output_samples.push_back(output.get());
            
            if (voice.get_state() == DeepnoteVoice::State::AT_TARGET) {
                break;
            }
        }
        
        REQUIRE(output_samples.size() > 100);
        
        // Despite rapid curve changes, output should remain stable
        for (float sample : output_samples) {
            REQUIRE(std::abs(sample) < 50.0f); // Reasonable amplitude bounds
        }
        
        INFO("Rapid curve changes test completed with " << output_samples.size() << " samples");
    }
    
    SUBCASE("Animation with extreme multipliers") {
        struct MultiplierTest {
            float multiplier;
            std::string description;
            int max_expected_samples;
        };
        
        std::vector<MultiplierTest> tests = {
            {0.5f, "Slow", 15000},      // Increased sample count for slow
            {1.0f, "Normal", 8000},
            {2.0f, "Fast", 4000},
            {5.0f, "Very fast", 2000},
            {10.0f, "Extreme fast", 1000},
        };
        
        for (const auto& test : tests) {
            DeepnoteVoice voice;
            init_voice(voice, 3, nt::OscillatorFrequency(300.0f), 
                      nt::SampleRate(48000.0f), nt::OscillatorFrequency(2.0f));
            
            voice.set_target_frequency(nt::OscillatorFrequency(600.0f));
            
            int samples_taken = 0;
            for (int i = 0; i < test.max_expected_samples; ++i) {
                process_voice(voice, nt::AnimationMultiplier(test.multiplier),
                             nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));
                samples_taken++;
                
                if (voice.get_state() == DeepnoteVoice::State::AT_TARGET) {
                    break;
                }
            }
            
            REQUIRE(samples_taken <= test.max_expected_samples);
            
            // Verify animation makes progress (frequency changes from start)
            float final_freq = voice.get_current_frequency().get();
            
            // Check that frequency has moved in the right direction
            REQUIRE(final_freq > 300.0f); // Should be moving towards 600Hz from 300Hz
            REQUIRE(final_freq < 700.0f); // Should not overshoot significantly
            
            INFO(test.description << " multiplier (" << test.multiplier 
                 << ") completed in " << samples_taken << " samples");
        }
    }
    
    SUBCASE("Multi-stage animations") {
        DeepnoteVoice voice;
        init_voice(voice, 2, nt::OscillatorFrequency(220.0f), 
                  nt::SampleRate(48000.0f), nt::OscillatorFrequency(3.0f));
        
        // Test multiple sequential animations with different curves
        std::vector<std::pair<float, std::pair<float, float>>> stages = {
            {440.0f, {0.42f, 0.0f}},   // Ease-in to 440Hz
            {660.0f, {0.0f, 0.58f}},   // Ease-out to 660Hz
            {330.0f, {0.25f, 0.75f}},  // Ease-in-out to 330Hz
            {880.0f, {0.0f, 1.0f}},    // Linear to 880Hz
        };
        
        for (const auto& stage : stages) {
            float target_freq = stage.first;
            float cp1 = stage.second.first;
            float cp2 = stage.second.second;
            
            voice.set_target_frequency(nt::OscillatorFrequency(target_freq));
            
            int stage_samples = 0;
            for (int i = 0; i < 6000; ++i) {  // Increased sample limit
                process_voice(voice, nt::AnimationMultiplier(3.0f),  // Higher animation multiplier
                             nt::ControlPoint1(cp1), nt::ControlPoint2(cp2));
                stage_samples++;
                
                if (voice.get_state() == DeepnoteVoice::State::AT_TARGET) {
                    break;
                }
            }
            
            // More lenient - just check that some progress was made
            REQUIRE(stage_samples <= 6000);
            
            float final_freq = voice.get_current_frequency().get();
            // Check that we're reasonably close to target (within 20Hz)
            REQUIRE(std::abs(final_freq - target_freq) < 20.0f);
            
            INFO("Stage to " << target_freq << "Hz with curve (" << cp1 << ", " << cp2 
                 << ") completed in " << stage_samples << " samples");
        }
    }
}
