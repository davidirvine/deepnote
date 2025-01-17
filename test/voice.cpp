#include "voice/deepnotevoice.hpp"
#include "ranges/range.hpp"
#include <doctest/doctest.h>
#include <random>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <memory>


namespace types = deepnote::nt;

struct StdLibRandomFloatGenerator {
    float operator()(float low, float high) const {
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

    OfstreamCsvTraceFunctor(const OfstreamCsvTraceFunctor& other) = delete;
    OfstreamCsvTraceFunctor& operator=(const OfstreamCsvTraceFunctor& other) = delete;
    OfstreamCsvTraceFunctor(OfstreamCsvTraceFunctor&& other) = delete;
    OfstreamCsvTraceFunctor& operator=(OfstreamCsvTraceFunctor&& other) = delete;

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
        (*out) << "index, startFreq, targetFreq, currentFreq, in_state, out_state, animationLfo, shapedAnimation, animationFreq, oscValue" << std::endl;
    }

    void operator()(const deepnote::TraceValues& values) const
    {
        static uint64_t index = 0;
        (*out) << std::fixed << std::setprecision(4) 
            << ++index 
            << ", " << values.startFreq
            << ", " << values.targetFreq 
            << ", " << values.currentFrequency 
            << ", " << values.in_state 
            << ", " << values.out_state
            << ", " << values.animationLfo_value 
            << ", " << values.shapedAnimationValue 
            << ", " << values.animationFreq 
            << ", " << values.oscValue 
            << std::endl;
    }

    private:
        std::unique_ptr<std::ofstream> out;
};



using TestVoiceType = deepnote::DeepnoteVoice<2>;

TEST_CASE("DeepnoteVoice log single cycle") {
    SUBCASE("startFrequency < targetFrequency") {
        const StdLibRandomFloatGenerator randomFunctor;
        const OfstreamCsvTraceFunctor traceFunctor("./single_cycle_ascending.csv");
        const auto sampleRate = types::SampleRate(48000);
        TestVoiceType voice;

        voice.Init(
            types::OscillatorFrequency(400.f),
            sampleRate,
            types::OscillatorFrequency(1.f),
            randomFunctor
        );
        
        voice.SetTargetFrequency(types::OscillatorFrequency(20000.f));

        CHECK(!voice.IsAtTarget());
        for (int i = 0; i < sampleRate.get(); i++) {
            voice.Process(
                types::AnimationMultiplier(1.f),
                types::ControlPoint1(0.08f),
                types::ControlPoint2(0.5f),
                traceFunctor);
        }
        CHECK(voice.IsAtTarget());
    }

    SUBCASE("startFrequency > targetFrequency") {
        const StdLibRandomFloatGenerator randomFunctor;
        const OfstreamCsvTraceFunctor traceFunctor("./single_cycle_decending.csv");
        const types::SampleRate sampleRate{48000};
        TestVoiceType voice;
        
        voice.Init(
            types::OscillatorFrequency(20000.f),
            sampleRate,
            types::OscillatorFrequency(1.f),
            randomFunctor
        );

        voice.SetTargetFrequency(types::OscillatorFrequency(400.f));
        
        CHECK(!voice.IsAtTarget());
        for (int i = 0; i < sampleRate.get(); i++) {
            voice.Process(
                types::AnimationMultiplier(1.f),
                types::ControlPoint1(0.08f),
                types::ControlPoint2(0.5f),
                traceFunctor);
        }
        CHECK(voice.IsAtTarget());
    }
}

TEST_CASE("DeepnoteVoice log multiple cycles and targets") {
    const StdLibRandomFloatGenerator randomFunctor;
    const OfstreamCsvTraceFunctor traceFunctor("./multi_cycle.csv");
    const types::SampleRate sampleRate{48000};
    TestVoiceType voice;
    
    voice.Init(
        types::OscillatorFrequency(20000.f),
        sampleRate,
        types::OscillatorFrequency(1.f),
        randomFunctor
    );

    voice.SetTargetFrequency(types::OscillatorFrequency(400.f));
    
    CHECK(!voice.IsAtTarget());
    for (int i = 0; i < sampleRate.get(); i++) {
        voice.Process(
            types::AnimationMultiplier(1.f),
            types::ControlPoint1(0.08f),
            types::ControlPoint2(0.5f),
            traceFunctor);
    }
    CHECK(voice.IsAtTarget());

    voice.SetTargetFrequency(types::OscillatorFrequency(10000.f));

    CHECK(!voice.IsAtTarget());
    for (int i = 0; i < sampleRate.get(); i++) {
    voice.Process(
        types::AnimationMultiplier(1.f),
        types::ControlPoint1(0.08f),
        types::ControlPoint2(0.5f),
        traceFunctor);
    }
    CHECK(voice.IsAtTarget());
    
    voice.SetTargetFrequency(types::OscillatorFrequency(6000.f));

    CHECK(!voice.IsAtTarget());
    for (int i = 0; i < sampleRate.get(); i++) {
    voice.Process(
        types::AnimationMultiplier(1.f),
        types::ControlPoint1(0.08f),
        types::ControlPoint2(0.5f),
        traceFunctor);
    }
    CHECK(voice.IsAtTarget());
}

TEST_CASE("DeepnoteVoice log target changed mid cycle") {
    const StdLibRandomFloatGenerator randomFunctor;
    const OfstreamCsvTraceFunctor traceFunctor("./target_change_mid_cycle.csv");
    const types::SampleRate sampleRate{48000};
    TestVoiceType voice;
    
    voice.Init(
        types::OscillatorFrequency(20000.f),
        sampleRate,
        types::OscillatorFrequency(1.f),
        randomFunctor
    );

    voice.SetTargetFrequency(types::OscillatorFrequency(400.f));
    
    CHECK(!voice.IsAtTarget());
    for (int i = 0; i < sampleRate.get() / 2; i++) {
        voice.Process(
            types::AnimationMultiplier(1.f),
            types::ControlPoint1(0.08f),
            types::ControlPoint2(0.5f),
            traceFunctor);
    }
    CHECK(!voice.IsAtTarget());

    voice.SetTargetFrequency(types::OscillatorFrequency(10000.f));

    CHECK(!voice.IsAtTarget());
    for (int i = 0; i < sampleRate.get(); i++) {
    voice.Process(
        types::AnimationMultiplier(1.f),
        types::ControlPoint1(0.08f),
        types::ControlPoint2(0.5f),
        traceFunctor);
    }
    CHECK(voice.IsAtTarget());
}