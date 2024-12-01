#include "range.hpp"
#include <doctest/doctest.h>


TEST_CASE("Range") {
  SUBCASE("Range::Range") {
    Range range(0, 10);
    CHECK(range.getLow() == 0);
    CHECK(range.getHigh() == 10);
  }

  SUBCASE("RangeSpansPositive") {
    Range range(0, 10);
    CHECK(range.length() == 10);
  }

  SUBCASE("RangeSpansZero") {
    Range range(-10, 10);
    CHECK(range.length() == 20);
  }

  SUBCASE("RangeSpansNegative") {
    Range range(-10, -5);
    CHECK(range.length() == 5);
  }
}

