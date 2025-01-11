#include "ranges/scaler.hpp"
#include <doctest/doctest.h>


TEST_CASE("Scaler") {

    SUBCASE("Scaler::scale") {
        deepnote::InputRange input(deepnote::Range(deepnote::RangeLow(0), deepnote::RangeHigh(10)));
        deepnote::OutputRange output(deepnote::Range(deepnote::RangeLow(0), deepnote::RangeHigh(10)));
        deepnote::Scaler scaler(input, output);
        CHECK(scaler(0) == 0);
        CHECK(scaler(10) == 10);
    }

    SUBCASE("Scaler::scale") {
        deepnote::InputRange input(deepnote::Range(deepnote::RangeLow(0), deepnote::RangeHigh(10)));
        deepnote::OutputRange output(deepnote::Range(deepnote::RangeLow(0), deepnote::RangeHigh(1)));
        deepnote::Scaler scaler(input, output);
        CHECK(scaler(0) == 0);
        CHECK(scaler(10) == 1);
    }

    SUBCASE("Scaler::scale") {
        deepnote::InputRange input(deepnote::Range(deepnote::RangeLow(-5.0), deepnote::RangeHigh(5)));
        deepnote::OutputRange output(deepnote::Range(deepnote::RangeLow(0), deepnote::RangeHigh(1)));
        deepnote::Scaler scaler(input, output);
        CHECK(scaler(-5) == 0);
        CHECK(scaler(5) == 1);
    }

    SUBCASE("Scaler::scale") {
        deepnote::InputRange input(deepnote::Range(deepnote::RangeLow(0), deepnote::RangeHigh(1)));
        deepnote::OutputRange output(deepnote::Range(deepnote::RangeLow(-5), deepnote::RangeHigh(5)));
        deepnote::Scaler scaler(input, output);
        CHECK(scaler(0.5) == 0);
        CHECK(scaler(0) == -5);
    }
}