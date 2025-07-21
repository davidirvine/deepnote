/**
 * @file basic_usage.cpp
 * @brief Basic usage example of the deepnote synthesizer voice
 * 
 * This example demonstrates how to create and use a DeepnoteVoice
 * to generate the classic THX Deep Note effect.
 */

#include "voice/deepnotevoice.hpp"
#include <iostream>
#include <fstream>
#include <vector>

using namespace deepnote;

int main() {
    // Audio configuration
    constexpr float SAMPLE_RATE = 48000.0f;
    constexpr float ANIMATION_DURATION_SECONDS = 2.0f;
    constexpr size_t TOTAL_SAMPLES = static_cast<size_t>(SAMPLE_RATE * ANIMATION_DURATION_SECONDS);
    
    // Create and initialize a voice
    DeepnoteVoice voice;
    
    // Initialize with 3 oscillators, starting at 200Hz, with 1Hz animation LFO
    init_voice(
        voice,
        3,                                          // Number of oscillators
        nt::OscillatorFrequency(200.0f),           // Start frequency
        nt::SampleRate(SAMPLE_RATE),               // Sample rate
        nt::OscillatorFrequency(1.0f / ANIMATION_DURATION_SECONDS)  // Animation speed
    );
    
    // Set target frequency for the Deep Note sweep
    voice.set_target_frequency(nt::OscillatorFrequency(8000.0f));
    
    // Add detuning for richness
    voice.detune_oscillators(nt::DetuneHz(2.5f));
    
    // Process audio samples
    std::vector<float> audio_output;
    audio_output.reserve(TOTAL_SAMPLES);
    
    std::cout << "Generating " << ANIMATION_DURATION_SECONDS << " seconds of Deep Note audio..." << std::endl;
    
    for (size_t sample = 0; sample < TOTAL_SAMPLES; ++sample) {
        // Process one sample with Bezier curve shaping
        auto output = process_voice(
            voice,
            nt::AnimationMultiplier(1.0f),     // Normal animation speed
            nt::ControlPoint1(0.1f),           // Slow start
            nt::ControlPoint2(0.9f)            // Fast finish
        );
        
        audio_output.push_back(output.get() * 0.1f);  // Scale down for safety
        
        // Progress indicator
        if (sample % (TOTAL_SAMPLES / 10) == 0) {
            std::cout << "Progress: " << (sample * 100 / TOTAL_SAMPLES) << "%" << std::endl;
        }
    }
    
    // Save to WAV file (simplified header)
    std::ofstream wav_file("deepnote_output.raw", std::ios::binary);
    if (wav_file.is_open()) {
        wav_file.write(reinterpret_cast<const char*>(audio_output.data()), 
                      audio_output.size() * sizeof(float));
        wav_file.close();
        std::cout << "Audio saved to deepnote_output.raw" << std::endl;
        std::cout << "Convert with: ffmpeg -f f32le -ar " << SAMPLE_RATE 
                  << " -ac 1 -i deepnote_output.raw deepnote_output.wav" << std::endl;
    }
    
    std::cout << "Deep Note generation complete!" << std::endl;
    return 0;
}
