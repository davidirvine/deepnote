#include "bezier.hpp"
#include <doctest/doctest.h>


TEST_CASE("BezierUnitShaper::shape") {
    BezierUnitShaper bezier(0.0, 1.0);
    CHECK(bezier.shape(0.0) == 0);
    CHECK(bezier.shape(1.0) == 1);
}