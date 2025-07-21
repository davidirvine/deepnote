#include "ranges/scaler.hpp"
#include <doctest/doctest.h>

namespace nt = deepnote::nt;

TEST_CASE("Scaler")
{
    SUBCASE("Scaler::scale")
    {
        nt::InputRange   input(deepnote::Range(nt::RangeLow(0), nt::RangeHigh(10)));
        nt::OutputRange  output(deepnote::Range(nt::RangeLow(0), nt::RangeHigh(10)));
        deepnote::Scaler scaler(input, output);
        CHECK(scaler(0) == 0);
        CHECK(scaler(10) == 10);
    }

    SUBCASE("Scaler::scale")
    {
        nt::InputRange   input(deepnote::Range(nt::RangeLow(0), nt::RangeHigh(10)));
        nt::OutputRange  output(deepnote::Range(nt::RangeLow(0), nt::RangeHigh(1)));
        deepnote::Scaler scaler(input, output);
        CHECK(scaler(0) == 0);
        CHECK(scaler(10) == 1);
    }

    SUBCASE("Scaler::scale")
    {
        nt::InputRange   input(deepnote::Range(nt::RangeLow(-5.0), nt::RangeHigh(5)));
        nt::OutputRange  output(deepnote::Range(nt::RangeLow(0), nt::RangeHigh(1)));
        deepnote::Scaler scaler(input, output);
        CHECK(scaler(-5) == 0);
        CHECK(scaler(5) == 1);
    }

    SUBCASE("Scaler::scale")
    {
        nt::InputRange   input(deepnote::Range(nt::RangeLow(0), nt::RangeHigh(1)));
        nt::OutputRange  output(deepnote::Range(nt::RangeLow(-5), nt::RangeHigh(5)));
        deepnote::Scaler scaler(input, output);
        CHECK(scaler(0.5) == 0);
        CHECK(scaler(0) == -5);
    }
}
