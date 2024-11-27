#include "deepnotevoice.hpp"
#include "deepnotevoice_impl.hpp"
#include "range.hpp"
#include <gtest/gtest.h>
#include <random>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace deepnotedrone;
using namespace std;

struct StdLibRandomFloatGenerator {
    static float GetRandomFloat(float low, float high) {
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

    void open(const string& filename) {
        out.open(filename);
        out << "index, start_low, start_high, targetFreq, in_state, out_state, animationLfo, shapedAnimation, animationFreq, frequency, oscValue" << endl;
    }

    void trace(const Range startRange, const float targetFreq, const int in_state, const int out_state, 
        const float animationLfo_value, const float shapedAnimationValue, const float animationFreq,
        const float frequency, const float oscValue) 
    {
        static uint64_t index = 0;
        out << fixed << setprecision(4) 
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
            << endl;
    }

    private:
        ofstream out;
};


using TestVoiceType = DeepnoteVoice<StdLibRandomFloatGenerator, 2, OfstreamCsvTrace>;

TEST(VoiceTest, LogSingleCycle) {
    const float sampleRate{48000};
    const Range startFrequencyRange{200.0f, 400.0f};
    const float targetFrequency{20000.0f};

    OfstreamCsvTrace trace;
    trace.open("./single_cycle.csv");

    TestVoiceType voice(startFrequencyRange, targetFrequency);
    voice.SetTracer(&trace);

    voice.Init(sampleRate, 1.0f);
    for (int i = 0; i < sampleRate; i++) {
        voice.Process();
    }
}
