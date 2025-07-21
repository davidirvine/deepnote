# Performance Guide

## Overview

The deepnote library is designed for real-time audio processing. This guide covers performance considerations and optimization strategies.

## Memory Usage

### Static Memory Allocation
- All voice data structures use fixed-size arrays to avoid runtime allocation
- Maximum oscillator count: `MAX_OSCILLATORS` (currently 16)
- Memory footprint per voice: ~2KB

### Recommended Patterns
```cpp
// ✅ Good: Pre-allocate voices
std::array<DeepnoteVoice, 8> voice_pool;

// ❌ Avoid: Runtime allocation in audio thread
auto voice = std::make_unique<DeepnoteVoice>();
```

## CPU Performance

### Hot Path Optimization
The `process_voice()` function is called once per audio sample and must be optimized:

1. **LFO Processing**: ~10 CPU cycles
2. **Bezier Shaping**: ~15 CPU cycles  
3. **Oscillator Processing**: ~20 cycles per oscillator
4. **State Management**: ~5 CPU cycles

**Total per voice**: ~50 + (20 × num_oscillators) CPU cycles

### Performance Tips

#### 1. Oscillator Count
```cpp
// For mobile/embedded: Use 1-3 oscillators
init_voice(voice, 2, start_freq, sample_rate, lfo_freq);

// For desktop: Can use up to 8-16 oscillators
init_voice(voice, 8, start_freq, sample_rate, lfo_freq);
```

#### 2. Animation Frequency
```cpp
// Slower animations use less CPU
nt::OscillatorFrequency slow_lfo(0.5f);  // 2 second animation
nt::OscillatorFrequency fast_lfo(4.0f);  // 0.25 second animation
```

#### 3. Bezier Complexity
```cpp
// Simple curves are faster
nt::ControlPoint1(0.0f), nt::ControlPoint2(1.0f)  // Linear (fastest)
nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f) // Mild curve
nt::ControlPoint1(0.1f), nt::ControlPoint2(0.9f)   // Strong curve
```

## Real-Time Constraints

### Buffer Sizes
Recommended audio buffer sizes for different scenarios:

- **Mobile devices**: 512-1024 samples
- **Desktop applications**: 256-512 samples  
- **Embedded hardware**: 128-256 samples
- **Professional audio**: 64-128 samples

### Latency Considerations
```cpp
// Calculate total latency
float buffer_latency_ms = (buffer_size / sample_rate) * 1000.0f;
float voice_processing_latency_ms = 0.02f;  // Typical processing delay
float total_latency_ms = buffer_latency_ms + voice_processing_latency_ms;
```

## Optimization Strategies

### 1. Voice Pooling
```cpp
class VoiceManager {
    std::array<DeepnoteVoice, MAX_VOICES> voices;
    std::bitset<MAX_VOICES> active_voices;
    
public:
    DeepnoteVoice* allocate_voice() {
        for (size_t i = 0; i < MAX_VOICES; ++i) {
            if (!active_voices[i]) {
                active_voices[i] = true;
                return &voices[i];
            }
        }
        return nullptr;  // No free voices
    }
};
```

### 2. Batch Processing
```cpp
// Process multiple samples at once for better cache locality
void process_voice_batch(DeepnoteVoice& voice, float* output, size_t num_samples) {
    for (size_t i = 0; i < num_samples; ++i) {
        output[i] = process_voice(voice, multiplier, cp1, cp2).get();
    }
}
```

### 3. Parameter Smoothing
```cpp
// Avoid parameter changes every sample
class SmoothedParameter {
    float current_value;
    float target_value;
    float smoothing_factor;
    
public:
    void set_target(float new_target) {
        target_value = new_target;
    }
    
    float process() {
        current_value += (target_value - current_value) * smoothing_factor;
        return current_value;
    }
};
```

## Platform-Specific Notes

### ARM Cortex-M (Daisy Seed)
- Use `-O3 -ffast-math` compiler flags
- Enable FPU: `-mfpu=fpv4-sp-d16 -mfloat-abi=hard`
- Consider fixed-point math for extreme optimization

### x86/x64 Desktop
- Use SSE/AVX instructions: `-march=native`
- Profile with tools like `perf` or Intel VTune
- Consider SIMD optimization for multiple voices

### Mobile (iOS/Android)
- Test on oldest target devices
- Monitor thermal throttling
- Use lower oscillator counts on older hardware

## Benchmarking

### Basic Performance Test
```cpp
#include <chrono>

void benchmark_voice_processing() {
    DeepnoteVoice voice;
    init_voice(voice, 4, nt::OscillatorFrequency(440.0f), 
               nt::SampleRate(48000.0f), nt::OscillatorFrequency(1.0f));
    
    const size_t num_samples = 48000;  // 1 second
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < num_samples; ++i) {
        process_voice(voice, nt::AnimationMultiplier(1.0f),
                     nt::ControlPoint1(0.25f), nt::ControlPoint2(0.75f));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    float real_time_factor = 1000000.0f / duration.count();
    std::cout << "Real-time factor: " << real_time_factor << "x" << std::endl;
}
```

A real-time factor > 1.0 means the voice can process audio faster than real-time.
