#include "unitshapers/linear.hpp"
#include <doctest/doctest.h>


TEST_CASE("LinearUnitShaper::shape") {
    deepnote::LinearUnitShaper linear;
    CHECK(linear.shape(0.0) == 0);
    CHECK(linear.shape(1.0) == 1);
}