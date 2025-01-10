#include "voice/deepnotevoice.hpp"
#include "voice/deepnotevoice_impl.hpp"
#include "ranges/range.hpp"
#include <doctest/doctest.h>
#include <random>
#include <iostream>
#include <fstream>
#include <iomanip>


struct StdLibRandomFloatGenerator {
    float GetFloat(float low, float high) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(low, high);
        return dis(gen);
    }
};

struct OfstreamCsvTrace {

    ~OfstreamCsvTrace() {
        out.close();
    }

    void open(const std::string& filename) {
        out.open(filename);
        out << "index, start_low, start_high, targetFreq, in_state, out_state, animationLfo, shapedAnimation, animationFreq, frequency, oscValue" << std::endl;
    }

    void trace(const deepnote::Range startRange, const float targetFreq, const int in_state, const int out_state, 
        const float animationLfo_value, const float shapedAnimationValue, const float animationFreq,
        const float frequency, const float oscValue) 
    {
        static uint64_t index = 0;
        out << std::fixed << std::setprecision(4) 
            << ++index 
            << ", " << startRange.getLow() << ", " << startRange.getHigh() 
            << ", " << targetFreq 
            << ", " << in_state 
            << ", " << out_state
            << ", " << animationLfo_value 
            << ", " << shapedAnimationValue 
            << ", " << animationFreq 
            << ", " << frequency 
            << ", " << oscValue 
            << std::endl;
    }

    private:
        std::ofstream out;
};

using TestVoiceType = deepnote::DeepnoteVoice<StdLibRandomFloatGenerator, 2, OfstreamCsvTrace>;

TEST_CASE("DeepnoteVoice log single cycle") {
    const float sampleRate{48000};
    const deepnote::Range startFrequencyRange{200.0f, 400.0f};
    const float targetFrequency{20000.0f};

    OfstreamCsvTrace trace;
    trace.open("./single_cycle.csv");

    TestVoiceType voice(startFrequencyRange, targetFrequency);
    voice.SetTracer(&trace);

    voice.Init(sampleRate, 1.0f);
    voice.TransitionToTarget();
    for (int i = 0; i < sampleRate; i++) {
        voice.Process();
    }
}

TEST_CASE("DeepnoteVoice log single cycle target frequency is less than start frequency") {
    const float sampleRate{48000};
    const deepnote::Range startFrequencyRange{200.0f, 400.0f};
    const float targetFrequency{100.0f};

    OfstreamCsvTrace trace;
    trace.open("./single_cycle-backwards.csv");

    TestVoiceType voice(startFrequencyRange, targetFrequency);
    voice.SetTracer(&trace);

    voice.Init(sampleRate, 1.0f);
    voice.TransitionToTarget();
    for (int i = 0; i < sampleRate; i++) {
        voice.Process();
    }
}


TEST_CASE("DeepnoteVoice log multi cycle") {
    const int sampleRate{48000};
    const deepnote::Range startFrequencyRange{200.0f, 400.0f};
    const float targetFrequency{20000.0f};

    OfstreamCsvTrace trace;
    trace.open("./multi_cycle.csv");

    TestVoiceType voice(startFrequencyRange, targetFrequency);
    voice.SetTracer(&trace);

    voice.Init(sampleRate, 1.0f);
    voice.TransitionToTarget();
    for (int i = 0; i < sampleRate * 4; i++) {
        voice.Process();
        if (voice.IsAtTarget()) {
            voice.TransitionToStart();
        } else if (voice.IsAtStart()) {
            voice.TransitionToTarget();
        }
    }
}
