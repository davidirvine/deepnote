#include "voice/deepnotevoice.hpp"
#include "ranges/range.hpp"
#include <doctest/doctest.h>
#include <random>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <memory>

namespace nt = deepnote::nt;

struct OfstreamCsvTraceFunctor
{
    OfstreamCsvTraceFunctor() = default;

    explicit OfstreamCsvTraceFunctor(const std::string &filename)
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
        (*out) << std::fixed << std::setprecision(4);
    }

    
    template <typename T, typename... Args>
    void operator()(T first, Args... rest) const
    {
        (*out) << first << ", ";
        (*this)(rest...);
    }

    template <typename T>
    void operator()(T value) const
    {
        (*out) << value << std::endl;
    }

private:

    std::unique_ptr<std::ofstream> out;
};

using TestVoiceType = deepnote::DeepnoteVoice<2>;

TEST_CASE("DeepnoteVoice log single cycle")
{
    SUBCASE("startFrequency < targetFrequency")
    {
        const OfstreamCsvTraceFunctor trace_functor("./single_cycle_ascending.csv");
        const auto sample_rate = nt::SampleRate(48000);
        TestVoiceType voice;

        voice.init(
            nt::OscillatorFrequency(400.f),
            sample_rate,
            nt::OscillatorFrequency(1.f));

        voice.set_target_frequency(nt::OscillatorFrequency(20000.f));

        CHECK(!voice.is_at_target());
        for (int i = 0; i < sample_rate.get(); i++)
        {
            voice.process(
                nt::AnimationMultiplier(1.f),
                nt::ControlPoint1(0.08f),
                nt::ControlPoint2(0.5f),
                trace_functor);
        }
        CHECK(voice.is_at_target());
    }

    SUBCASE("startFrequency > targetFrequency")
    {
        const OfstreamCsvTraceFunctor trace_functor("./single_cycle_decending.csv");
        const nt::SampleRate sample_rate{48000};
        TestVoiceType voice;

        voice.init(
            nt::OscillatorFrequency(20000.f),
            sample_rate,
            nt::OscillatorFrequency(1.f));

        voice.set_target_frequency(nt::OscillatorFrequency(400.f));

        CHECK(!voice.is_at_target());
        for (int i = 0; i < sample_rate.get(); i++)
        {
            voice.process(
                nt::AnimationMultiplier(1.f),
                nt::ControlPoint1(0.08f),
                nt::ControlPoint2(0.5f),
                trace_functor);
        }
        CHECK(voice.is_at_target());
    }
}

TEST_CASE("DeepnoteVoice log multiple cycles and targets")
{
    const OfstreamCsvTraceFunctor trace_functor("./multi_cycle.csv");
    const nt::SampleRate sample_rate{48000};
    TestVoiceType voice;

    voice.init(
        nt::OscillatorFrequency(20000.f),
        sample_rate,
        nt::OscillatorFrequency(1.f));

    voice.set_target_frequency(nt::OscillatorFrequency(400.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get(); i++)
    {
        voice.process(
            nt::AnimationMultiplier(1.f),
            nt::ControlPoint1(0.08f),
            nt::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(voice.is_at_target());

    voice.set_target_frequency(nt::OscillatorFrequency(10000.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get(); i++)
    {
        voice.process(
            nt::AnimationMultiplier(1.f),
            nt::ControlPoint1(0.08f),
            nt::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(voice.is_at_target());

    voice.set_target_frequency(nt::OscillatorFrequency(6000.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get(); i++)
    {
        voice.process(
            nt::AnimationMultiplier(1.f),
            nt::ControlPoint1(0.08f),
            nt::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(voice.is_at_target());
}

TEST_CASE("DeepnoteVoice log target changed mid cycle")
{
    const OfstreamCsvTraceFunctor trace_functor("./target_change_mid_cycle.csv");
    const nt::SampleRate sample_rate{48000};
    TestVoiceType voice;

    voice.init(
        nt::OscillatorFrequency(20000.f),
        sample_rate,
        nt::OscillatorFrequency(1.f));

    voice.set_target_frequency(nt::OscillatorFrequency(400.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get() / 2; i++)
    {
        voice.process(
            nt::AnimationMultiplier(1.f),
            nt::ControlPoint1(0.08f),
            nt::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(!voice.is_at_target());

    voice.set_target_frequency(nt::OscillatorFrequency(10000.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get(); i++)
    {
        voice.process(
            nt::AnimationMultiplier(1.f),
            nt::ControlPoint1(0.08f),
            nt::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(voice.is_at_target());
}

TEST_CASE("DeepnoteVoice log target changed mid cycle then reset")
{
    const OfstreamCsvTraceFunctor trace_functor("./target_change_reset.csv");
    const nt::SampleRate sample_rate{48000};
    TestVoiceType voice;

    voice.init(
        nt::OscillatorFrequency(20000.f),
        sample_rate,
        nt::OscillatorFrequency(1.f));

    voice.set_target_frequency(nt::OscillatorFrequency(400.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get() / 2; i++)
    {
        voice.process(
            nt::AnimationMultiplier(1.f),
            nt::ControlPoint1(0.08f),
            nt::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(!voice.is_at_target());

    voice.set_target_frequency(nt::OscillatorFrequency(10000.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get() / 2; i++)
    {
        voice.process(
            nt::AnimationMultiplier(1.f),
            nt::ControlPoint1(0.08f),
            nt::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(!voice.is_at_target());

    voice.set_start_frequency(nt::OscillatorFrequency(500.f));

    CHECK(!voice.is_at_target());
    for (int i = 0; i < sample_rate.get(); i++)
    {
        voice.process(
            nt::AnimationMultiplier(1.f),
            nt::ControlPoint1(0.08f),
            nt::ControlPoint2(0.5f),
            trace_functor);
    }
    CHECK(voice.is_at_target());
}
