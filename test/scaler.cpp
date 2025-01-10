#include "ranges/scaler.hpp"
#include <doctest/doctest.h>


TEST_CASE("Scaler") {

    SUBCASE("Scaler::scale") {
        deepnote::Range input(0, 10);
        deepnote::Range output(0, 10);
        deepnote::Scaler scaler(input, output);
        CHECK(scaler.Scale(0) == 0);
        CHECK(scaler.Scale(10) == 10);
    }

    SUBCASE("Scaler::scale") {
        deepnote::Range input(0, 10);
        deepnote::Range output(0, 1);
        deepnote::Scaler scaler(input, output);
        CHECK(scaler.Scale(0) == 0);
        CHECK(scaler.Scale(10) == 1);
    }

    SUBCASE("Scaler::scale") {
        deepnote::Range input(-5.0, 5);
        deepnote::Range output(0, 1);
        deepnote::Scaler scaler(input, output);
        CHECK(scaler.Scale(-5) == 0);
        CHECK(scaler.Scale(5) == 1);
    }

    SUBCASE("Scaler::scale") {
        deepnote::Range input(0, 1);
        deepnote::Range output(-5, 5);
        deepnote::Scaler scaler(input, output);
        CHECK(scaler.Scale(0.5) == 0);
        CHECK(scaler.Scale(0) == -5);
    }
}