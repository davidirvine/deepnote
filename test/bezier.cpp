#include "unitshapers/bezier.hpp"
#include <doctest/doctest.h>


TEST_CASE("BezierUnitShaper::shape") {
    deepnote::BezierUnitShaper bezier(deepnote::ControlPoint1(0.0), deepnote::ControlPoint2(1.0));
    CHECK(bezier(0.0) == 0);
    CHECK(bezier(1.0) == 1);
}