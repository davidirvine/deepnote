#include "voice/deepnotevoice.hpp"
#include "ranges/range.hpp"
#include <doctest/doctest.h>
#include <random>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <memory>


struct StdLibRandomFloatGenerator {
    float operator()(float low, float high) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(low, high);
        return dis(gen);
    }
};

struct OfstreamCsvTraceFunctor {

    OfstreamCsvTraceFunctor() = default;

    OfstreamCsvTraceFunctor(const std::string& filename) 
    {
        open(filename);
    }

    OfstreamCsvTraceFunctor(OfstreamCsvTraceFunctor&& other)
    {
        out.reset(other.out.get());
        other.out.release();
    }
    
    OfstreamCsvTraceFunctor& operator=(OfstreamCsvTraceFunctor&& other) 
    {
        if (this != &other) {
            close();
            out.reset(other.out.get());
            other.out.release();
        }
        return *this;
    }

    void close() 
    {
        if (out != nullptr) {
            out->close();
            out.release();
        }
    }

    void open(const std::string& filename) 
    {
        close();
        out.reset(new std::ofstream(filename));
        (*out) << "index, start_low, start_high, targetFreq, in_state, out_state, animationLfo, shapedAnimation, animationFreq, frequency, oscValue" << std::endl;
    }

    void operator()(const deepnote::TraceValues& values)
    {
        static uint64_t index = 0;
        (*out) << std::fixed << std::setprecision(4) 
            << ++index 
            << ", " << values.startRange.GetLow() << ", " << values.startRange.GetHigh() 
            << ", " << values.targetFreq 
            << ", " << values.in_state 
            << ", " << values.out_state
            << ", " << values.animationLfo_value 
            << ", " << values.shapedAnimationValue 
            << ", " << values.animationFreq 
            << ", " << values.frequency 
            << ", " << values.oscValue 
            << std::endl;
    }

    private:
        std::unique_ptr<std::ofstream> out;
};



using TestVoiceType = deepnote::DeepnoteVoice<2>;

TEST_CASE("DeepnoteVoice log single cycle") {
    const float sampleRate{48000};
    const deepnote::Range startFrequencyRange{deepnote::RangeLow(200.0f), deepnote::RangeHigh(400.0f)};
    const float targetFrequency{20000.0f};

    OfstreamCsvTraceFunctor traceFunctor("./single_cycle.csv");
    StdLibRandomFloatGenerator randomFunctor;
    TestVoiceType voice(startFrequencyRange, targetFrequency);

    voice.Init(sampleRate, 1.0f, deepnote::make_named<deepnote::RandomFunctorT>(randomFunctor));
    voice.TransitionToTarget();
    auto typedTraceFunctor = deepnote::make_named<deepnote::TraceFunctorT>([&traceFunctor](deepnote::TraceValues values){traceFunctor(values);});
    for (int i = 0; i < sampleRate; i++) {
        voice.Process(typedTraceFunctor);
    }
}

TEST_CASE("DeepnoteVoice log single cycle target frequency is less than start frequency") {
    const float sampleRate{48000};
    const deepnote::Range startFrequencyRange{deepnote::RangeLow(200.0f), deepnote::RangeHigh(400.0f)};
    const float targetFrequency{100.0f};

    OfstreamCsvTraceFunctor traceFunctor("./single_cycle-backwards.csv");
    StdLibRandomFloatGenerator randomFunctor;
    TestVoiceType voice(startFrequencyRange, targetFrequency);

    voice.Init(sampleRate, 1.0f, deepnote::make_named<deepnote::RandomFunctorT>(randomFunctor));
    voice.TransitionToTarget();

    auto typedTraceFunctor = deepnote::make_named<deepnote::TraceFunctorT>([&traceFunctor](deepnote::TraceValues values){traceFunctor(values);});
    
    for (int i = 0; i < sampleRate; i++) {
        voice.Process(typedTraceFunctor);
    }
}

TEST_CASE("DeepnoteVoice log multi cycle") {
    const int sampleRate{48000};
    const deepnote::Range startFrequencyRange{deepnote::RangeLow(200.0f), deepnote::RangeHigh(400.0f)};
    const float targetFrequency{20000.0f};

    StdLibRandomFloatGenerator randomFunctor;
    OfstreamCsvTraceFunctor traceFunctor("./multi_cycle.csv");
    TestVoiceType voice(startFrequencyRange, targetFrequency);

    voice.Init(sampleRate, 1.0f, deepnote::make_named<deepnote::RandomFunctorT>(randomFunctor));
    voice.TransitionToTarget();

    auto typedTraceFunctor = deepnote::make_named<deepnote::TraceFunctorT>([&traceFunctor](deepnote::TraceValues values){traceFunctor(values);});
    
    for (int i = 0; i < sampleRate * 4; i++) {
        voice.Process(typedTraceFunctor);
        if (voice.IsAtTarget()) {
            voice.TransitionToStart();
        } else if (voice.IsAtStart()) {
            voice.TransitionToTarget();
        }
    }
}
