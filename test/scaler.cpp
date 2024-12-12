#include "scaler.hpp"
#include <doctest/doctest.h>


TEST_CASE("Scaler") {

    SUBCASE("Scaler::scale") {
        deepnote::Range input(0, 10);
        deepnote::Range output(0, 10);
        deepnote::Scaler scaler(input, output);
        CHECK(scaler.scale(0) == 0);
        CHECK(scaler.scale(10) == 10);
    }

    SUBCASE("Scaler::scale") {
        deepnote::Range input(0, 10);
        deepnote::Range output(0, 1);
        deepnote::Scaler scaler(input, output);
        CHECK(scaler.scale(0) == 0);
        CHECK(scaler.scale(10) == 1);
    }

    SUBCASE("Scaler::scale") {
        deepnote::Range input(-5.0, 5);
        deepnote::Range output(0, 1);
        deepnote::Scaler scaler(input, output);
        CHECK(scaler.scale(-5) == 0);
        CHECK(scaler.scale(5) == 1);
    }

    SUBCASE("Scaler::scale") {
        deepnote::Range input(0, 1);
        deepnote::Range output(-5, 5);
        deepnote::Scaler scaler(input, output);
        CHECK(scaler.scale(0.5) == 0);
        CHECK(scaler.scale(0) == -5);
    }
}