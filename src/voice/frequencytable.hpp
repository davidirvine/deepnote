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
        { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f },
        { 1244.51f, 1046.50f, 587.33f, 523.25f, 392.00f, 130.81f, 98.00f, 65.41f, 32.70f },
        { 1318.51f, 1108.73f, 622.25f, 554.37f, 415.30f, 138.59f, 103.83f, 69.30f, 34.65f },
        { 1396.91f, 1174.66f, 659.26f, 587.33f, 440.00f, 146.83f, 110.00f, 73.42f, 36.71f },
        { 1479.98f, 1244.51f, 698.46f, 622.25f, 466.16f, 155.56f, 116.54f, 77.78f, 38.89f },
        { 1567.98f, 1318.51f, 739.99f, 659.26f, 493.88f, 164.81f, 123.47f, 82.41f, 41.20f },
        { 1661.22f, 1396.91f, 783.99f, 698.46f, 523.25f, 174.61f, 130.81f, 87.31f, 43.65f },
        { 1760.00f, 1479.98f, 830.61f, 739.99f, 554.37f, 185.00f, 138.59f, 92.50f, 46.25f },
        { 1864.66f, 1567.98f, 880.00f, 783.99f, 587.33f, 196.00f, 146.83f, 98.00f, 49.00f },
        { 1975.53f, 1661.22f, 932.33f, 830.61f, 622.25f, 207.65f, 155.56f, 103.83f, 51.91f },
        { 2093.00f, 1760.00f, 987.77f, 880.00f, 659.26f, 220.00f, 164.81f, 110.00f, 55.00f },
        { 2217.46f, 1864.66f, 1046.50f, 932.33f, 698.46f, 233.08f, 174.61f, 116.54f, 58.27f },
        { 2349.32f, 1975.53f, 1108.73f, 987.77f, 739.99f, 246.94f, 185.00f, 123.47f, 61.74f },
    };
};

} // namespace deepnote