#include "voice/frequencytable.hpp"
#include <doctest/doctest.h>
#include <random>


namespace types = deepnote::nt;

struct StdLibRandomFloatGenerator {
    float operator()(float low, float high) const {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(low, high);
        return dis(gen);
    }
};

TEST_CASE("FrequencyTable") {
  SUBCASE("FrequencyTable::Init") {
    const StdLibRandomFloatGenerator randomFunctor;
    const types::RangeLow low(200.0f);
    const types::RangeHigh high(400.0f);
    const types::OscillatorFrequencyRange startFrequencyRange(deepnote::Range(low, high));
    deepnote::FrequencyTable table;
    table.initialize(startFrequencyRange, randomFunctor);
    table.set_current_index(types::FrequencyTableIndex(0));

    for (int i = 0; i < deepnote::NUM_VOICES; ++i)
    {
        const auto frequency = table.get_frequency(types::VoiceIndex(i));
        CHECK(frequency.get() >= low.get());
        CHECK(frequency.get() <= high.get());
    }
  }
}
