#include "voice/deepnotevoice.hpp"
#include "ranges/range.hpp"
#include <doctest/doctest.h>
#include <random>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <memory>

namespace types = deepnote::nt;

struct StdLibRandomFloatGenerator
{
    float operator()(float low, float high) const
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(low, high);
        return dis(gen);
    }
};

struct OfstreamCsvTraceFunctor
{

    OfstreamCsvTraceFunctor() = default;

    OfstreamCsvTraceFunctor(const std::string &filename)
    {
        open(filename);
    }

    OfstreamCsvTraceFunctor(const OfstreamCsvTraceFunctor &other) = delete;
    OfstreamCsvTraceFunctor &operator=(const OfstreamCsvTraceFunctor &other) = delete;
    OfstreamCsvTraceFunctor(OfstreamCsvTraceFunctor &&other) = delete;
    OfstreamCsvTraceFunctor &operator=(OfstreamCsvTraceFunctor &&other) = delete;

    void close()
    {
        if (out != nullptr)
        {
            out->close();
            out.release();
        }
    }

    void open(const std::string &filename)
    {
        close();
        out.reset(new std::ofstream(filename));
        (*out) << "index, startFreq, targetFreq, currentFreq, in_state, out_state, animationLfo, shapedAnimation, animationFreq, oscValue" << std::endl;
    }

    void operator()(const deepnote::TraceValues &values) const
    {
        static uint64_t index = 0;
        (*out) << std::fixed << std::setprecision(4)
               << ++index
               << ", " << values.start_freq
               << ", " << values.target_freq
               << ", " << values.current_frequency
               << ", " << values.in_state
               << ", " << values.out_state
               << ", " << values.animation_lfo_value
               << ", " << values.shaped_animation_value
               << ", " << values.animation_freq
               << ", " << values.osc_value
               << std::endl;
    }

private:
    std::unique_ptr<std::ofstream> out;
};

using TestVoiceType = deepnote::DeepnoteVoice<2>;

TEST_CASE("DeepnoteVoice log single cycle")
{
    SUBCASE("startFrequency < targetFrequency")
    {
        const StdLibRandomFloatGenerator random_functor;
        const OfstreamCsvTraceFunctor trace_functor("./single_cycle_ascending.csv");
        const auto sample_rate = types::SampleRate(48000);
        TestVoiceType voice;

        voice.init(
            types::OscillatorFrequency(400.f),
            sample_rate,
            types::OscillatorFrequency(1.f),
            random_functor);

        voice.set_target_frequency(types::OscillatorFrequency(20000.f));

        CHECK(!voice.is_at_target());
        for (int i = 0; i < sample_rate.get(); i++)
        {
            voice.process(
                types::AnimationMultiplier(1.f),
                types::ControlPoint1(0.08f),
                types::ControlPoint2(0.5f),
                trace_functor);
        }
        CHECK(voice.is_at_target());
    }

    SUBCASE("startFrequency > targetFrequency")
    {
        const StdLibRandomFloatGenerator random_functor;
        const OfstreamCsvTraceFunctor trace_functor("./single_cycle_decending.csv");
        const types::SampleRate sample_rate{48000};
        TestVoiceType voice;

        voice.init(
            types::OscillatorFrequency(20000.f),
            sample_rate,
            types::OscillatorFrequency(1.f),
            random_functor);

        voice.set_target_frequency(types::OscillatorFrequency(400.f));

        CHECK(!voice.is_at_target());
        for (int i = 0; i < sample_rate.get(); i++)
        {
            voice.process(
                types::AnimationMultiplier(1.f),
                types::ControlPoint1(0.08f),
                types::ControlPoint2(0.5f),
                trace_functor);
        }
        CHECK(voice.is_at_target());
    }
}

TEST_CASE("DeepnoteVoice log multiple cycles and targets")
{
    const StdLibRandomFloatGenerator random_functor;
    const OfstreamCsvTraceFunctor trace_functor("./multi_cycle.csv");
    const types::SampleRate sample_rate{48000};
    TestVoiceType voice;

    voice.init(
        types::OscillatorFrequency(20000.f),
        sample_rate,
        types::OscillatorFrequency(1.f),
        random_functor);

    voice.set_target_frequency(types::OscillatorFrequency(400.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get(); i++)
    {
        voice.process(
            types::AnimationMultiplier(1.f),
            types::ControlPoint1(0.08f),
            types::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(voice.is_at_target());

    voice.set_target_frequency(types::OscillatorFrequency(10000.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get(); i++)
    {
        voice.process(
            types::AnimationMultiplier(1.f),
            types::ControlPoint1(0.08f),
            types::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(voice.is_at_target());

    voice.set_target_frequency(types::OscillatorFrequency(6000.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get(); i++)
    {
        voice.process(
            types::AnimationMultiplier(1.f),
            types::ControlPoint1(0.08f),
            types::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(voice.is_at_target());
}

TEST_CASE("DeepnoteVoice log target changed mid cycle")
{
    const StdLibRandomFloatGenerator random_functor;
    const OfstreamCsvTraceFunctor trace_functor("./target_change_mid_cycle.csv");
    const types::SampleRate sample_rate{48000};
    TestVoiceType voice;

    voice.init(
        types::OscillatorFrequency(20000.f),
        sample_rate,
        types::OscillatorFrequency(1.f),
        random_functor);

    voice.set_target_frequency(types::OscillatorFrequency(400.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get() / 2; i++)
    {
        voice.process(
            types::AnimationMultiplier(1.f),
            types::ControlPoint1(0.08f),
            types::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(!voice.is_at_target());

    voice.set_target_frequency(types::OscillatorFrequency(10000.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get(); i++)
    {
        voice.process(
            types::AnimationMultiplier(1.f),
            types::ControlPoint1(0.08f),
            types::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(voice.is_at_target());
}

TEST_CASE("DeepnoteVoice log target changed mid cycle then reset")
{
    const StdLibRandomFloatGenerator random_functor;
    const OfstreamCsvTraceFunctor trace_functor("./target_change_reset.csv");
    const types::SampleRate sample_rate{48000};
    TestVoiceType voice;

    voice.init(
        types::OscillatorFrequency(20000.f),
        sample_rate,
        types::OscillatorFrequency(1.f),
        random_functor);

    voice.set_target_frequency(types::OscillatorFrequency(400.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get() / 2; i++)
    {
        voice.process(
            types::AnimationMultiplier(1.f),
            types::ControlPoint1(0.08f),
            types::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(!voice.is_at_target());

    voice.set_target_frequency(types::OscillatorFrequency(10000.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get() / 2; i++)
    {
        voice.process(
            types::AnimationMultiplier(1.f),
            types::ControlPoint1(0.08f),
            types::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(!voice.is_at_target());

    voice.reset_start_frequency(types::OscillatorFrequency(500.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get(); i++)
    {
        voice.process(
            types::AnimationMultiplier(1.f),
            types::ControlPoint1(0.08f),
            types::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(voice.is_at_target());
}
