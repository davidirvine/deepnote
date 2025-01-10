#include "ranges/range.hpp"
#include <doctest/doctest.h>


TEST_CASE("Range") {
  SUBCASE("Range::Range") {
    deepnote::Range range(0, 10);
    CHECK(range.GetLow() == 0);
    CHECK(range.GetHigh() == 10);
  }

  SUBCASE("RangeSpansPositive") {
    deepnote::Range range(0, 10);
    CHECK(range.Length() == 10);
  }

  SUBCASE("RangeSpansZero") {
    deepnote::Range range(-10, 10);
    CHECK(range.Length() == 20);
  }

  SUBCASE("RangeSpansNegative") {
    deepnote::Range range(-10, -5);
    CHECK(range.Length() == 5);
  }
}

