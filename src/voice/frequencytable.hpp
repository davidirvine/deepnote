#pragma once

#include "ranges/range.hpp"

namespace deepnote
{

constexpr int NUM_TABLES = 13;
constexpr int NUM_TRIO_VOICES = 4;
constexpr int NUM_DUO_VOICES = 5;
constexpr int NUM_VOICES = NUM_TRIO_VOICES + NUM_DUO_VOICES;

namespace nt
{
    using OscillatorFrequency = NamedType<float, struct OscillatorFrequencyTag>;
    using OscillatorFrequencyRange = NamedType<Range, struct OscillatorFrequencyRangeTag>;
    using FrequencyTableIndex = NamedType<int, struct FrequencyTableIndexTag>;
    using VoiceIndex = NamedType<int, struct VoiceIndexTag>;
}

struct FrequencyTable
{
    FrequencyTable()
    {}

    virtual ~FrequencyTable()
    {}

    template <typename F>
    void initialize(const nt::OscillatorFrequencyRange& range, const F& random)
    {
        for (int i = 0; i < NUM_VOICES; ++i) {
            frequencies[0][i] = random(range.get().GetLow().get(), range.get().GetHigh().get());
        }
    }

    bool setCurrentIndex(const nt::FrequencyTableIndex& index)
    {
        const bool changed = currentIndex != index.get();
        this->currentIndex = index.get() % NUM_TABLES;
        return changed;
    }
    
    nt::OscillatorFrequency getFrequency(const nt::VoiceIndex& index) const
    {
        return nt::OscillatorFrequency(frequencies[currentIndex][index.get() % NUM_VOICES]);
    }

private:
    int currentIndex{0};

    float frequencies[NUM_TABLES][NUM_VOICES] = {
        {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f},
        {1396.91f, 1174.66f, 659.25f, 587.33f, 440.00f, 146.83f, 110.0f, 73.42f, 36.71}, 
        {1396.91f, 1174.66f, 659.25f, 587.33f, 440.00f, 146.83f, 110.0f, 73.42f, 36.71}, 
        {1396.91f, 1174.66f, 659.25f, 587.33f, 440.00f, 146.83f, 110.0f, 73.42f, 36.71}, 
        {1396.91f, 1174.66f, 659.25f, 587.33f, 440.00f, 146.83f, 110.0f, 73.42f, 36.71}, 
        {1396.91f, 1174.66f, 659.25f, 587.33f, 440.00f, 146.83f, 110.0f, 73.42f, 36.71}, 
        {1396.91f, 1174.66f, 659.25f, 587.33f, 440.00f, 146.83f, 110.0f, 73.42f, 36.71}, 
        {1396.91f, 1174.66f, 659.25f, 587.33f, 440.00f, 146.83f, 110.0f, 73.42f, 36.71}, 
        {1396.91f, 1174.66f, 659.25f, 587.33f, 440.00f, 146.83f, 110.0f, 73.42f, 36.71}, 
        {1396.91f, 1174.66f, 659.25f, 587.33f, 440.00f, 146.83f, 110.0f, 73.42f, 36.71}, 
        {1396.91f, 1174.66f, 659.25f, 587.33f, 440.00f, 146.83f, 110.0f, 73.42f, 36.71}, 
        {1396.91f, 1174.66f, 659.25f, 587.33f, 440.00f, 146.83f, 110.0f, 73.42f, 36.71}, 
        {1396.91f, 1174.66f, 659.25f, 587.33f, 440.00f, 146.83f, 110.0f, 73.42f, 36.71}
    };
};

} // namespace deepnote