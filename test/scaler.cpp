#include "ranges/scaler.hpp"
#include <doctest/doctest.h>

namespace types = deepnote::nt;

TEST_CASE("Scaler")
{
    SUBCASE("Scaler::scale")
    {
        types::InputRange input(deepnote::Range(types::RangeLow(0), types::RangeHigh(10)));
        types::OutputRange output(deepnote::Range(types::RangeLow(0), types::RangeHigh(10)));
        deepnote::Scaler scaler(input, output);
        CHECK(scaler(0) == 0);
        CHECK(scaler(10) == 10);
    }

    SUBCASE("Scaler::scale")
    {
        types::InputRange input(deepnote::Range(types::RangeLow(0), types::RangeHigh(10)));
        types::OutputRange output(deepnote::Range(types::RangeLow(0), types::RangeHigh(1)));
        deepnote::Scaler scaler(input, output);
        CHECK(scaler(0) == 0);
        CHECK(scaler(10) == 1);
    }

    SUBCASE("Scaler::scale")
    {
        types::InputRange input(deepnote::Range(types::RangeLow(-5.0), types::RangeHigh(5)));
        types::OutputRange output(deepnote::Range(types::RangeLow(0), types::RangeHigh(1)));
        deepnote::Scaler scaler(input, output);
        CHECK(scaler(-5) == 0);
        CHECK(scaler(5) == 1);
    }

    SUBCASE("Scaler::scale")
    {
        types::InputRange input(deepnote::Range(types::RangeLow(0), types::RangeHigh(1)));
        types::OutputRange output(deepnote::Range(types::RangeLow(-5), types::RangeHigh(5)));
        deepnote::Scaler scaler(input, output);
        CHECK(scaler(0.5) == 0);
        CHECK(scaler(0) == -5);
    }
}
