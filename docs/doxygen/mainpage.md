@mainpage Deepnote Synthesizer Voice Library

@section intro Introduction

The deepnote library provides a C++14 header-only implementation of a 
synthesizer voice inspired by the iconic THX Deep Note sound effect.

@section features Key Features

- **Header-only library**: No compilation required, just include and use
- **Real-time performance**: Optimized for audio applications
- **Multiple oscillators**: Support for up to 16 detuned oscillators per voice
- **Bezier curve shaping**: Smooth, controllable frequency transitions
- **Strong typing**: Type-safe API prevents common audio programming errors
- **Comprehensive testing**: Full test coverage with doctest framework

@section quick_start Quick Start

@code{cpp}
#include "voice/deepnotevoice.hpp"

using namespace deepnote;

// Create and initialize a voice
DeepnoteVoice voice;
init_voice(voice, 3, nt::OscillatorFrequency(200.0f), 
          nt::SampleRate(48000.0f), nt::OscillatorFrequency(0.5f));

// Set target frequency for animation
voice.set_target_frequency(nt::OscillatorFrequency(8000.0f));

// Process audio samples
for (size_t i = 0; i < num_samples; ++i) {
    auto output = process_voice(voice, nt::AnimationMultiplier(1.0f),
                               nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));
    audio_buffer[i] = output.get();
}
@endcode

@section architecture Architecture Overview

The library is organized into several key components:

- **DeepnoteVoice**: Main voice class containing oscillators and state machine
- **State Machine**: Manages transitions between PENDING → IN_TRANSIT → AT_TARGET
- **LFO Animation**: Controls the speed and timing of frequency transitions  
- **Bezier Shaping**: Provides non-linear curve shapes for natural-sounding animations
- **Strong Types**: Type-safe wrappers around float values (frequencies, rates, etc.)

@section performance Performance Characteristics

- **CPU Usage**: ~50 + (20 × num_oscillators) cycles per sample
- **Memory Usage**: ~2KB per voice (fixed allocation)
- **Real-time Safe**: No dynamic allocation in audio processing paths
- **Scalable**: Support for 1-16 oscillators depending on performance requirements

@section examples Examples

See the `docs/examples/` directory for complete usage examples:
- `basic_usage.cpp` - Simple Deep Note generation
- `vcvrack_integration.cpp` - VCV Rack module integration
- `daisy_seed_integration.cpp` - Embedded hardware usage

@section testing Testing

The library includes comprehensive unit tests using the doctest framework:

@code{bash}
cd test
mkdir build && cd build
cmake .. && make
./bin/tests
@endcode

@section license License

This project is licensed under the MIT License - see LICENSE.txt for details.

@section acknowledgments Acknowledgments

- Inspired by the THX Deep Note sound effect
- Built on the DaisySP digital signal processing library
- Uses doctest for unit testing framework
