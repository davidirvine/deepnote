#include "unitshapers/bezier.hpp"
#include <doctest/doctest.h>

namespace nt = deepnote::nt;

TEST_CASE("BezierUnitShaper::shape")
{
    deepnote::BezierUnitShaper bezier(nt::ControlPoint1(0.0), nt::ControlPoint2(1.0));
    CHECK(bezier(0.0) == 0);
    CHECK(bezier(1.0) == 1);
}
