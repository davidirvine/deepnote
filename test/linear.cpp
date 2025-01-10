#include "unitshapers/linear.hpp"
#include <doctest/doctest.h>


TEST_CASE("LinearUnitShaper::shape") {
    deepnote::LinearUnitShaper linear;
    CHECK(linear.Shape(0.0) == 0);
    CHECK(linear.Shape(1.0) == 1);
}