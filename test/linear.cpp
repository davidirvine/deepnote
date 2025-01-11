#include "unitshapers/linear.hpp"
#include <doctest/doctest.h>


TEST_CASE("LinearUnitShaper::shape") {
    deepnote::LinearUnitShaper linear;
    CHECK(linear(0.0) == 0);
    CHECK(linear(1.0) == 1);
}