#include "unitshapers/bezier.hpp"
#include <doctest/doctest.h>

namespace types = deepnote::nt;

TEST_CASE("BezierUnitShaper::shape")
{
    deepnote::BezierUnitShaper bezier(types::ControlPoint1(0.0), types::ControlPoint2(1.0));
    CHECK(bezier(0.0) == 0);
    CHECK(bezier(1.0) == 1);
}
