#include "ranges/range.hpp"
#include <doctest/doctest.h>


TEST_CASE("Range") {
  SUBCASE("Range::Range") {
    deepnote::Range range(deepnote::RangeLow(0), deepnote::RangeHigh(10));
    CHECK(range.GetLow() == 0);
    CHECK(range.GetHigh() == 10);
  }

  SUBCASE("RangeSpansPositive") {
    deepnote::Range range(deepnote::RangeLow(0), deepnote::RangeHigh(10));
    CHECK(range.Length() == 10);
  }

  SUBCASE("RangeSpansZero") {
    deepnote::Range range(deepnote::RangeLow(-10), deepnote::RangeHigh(10));
    CHECK(range.Length() == 20);
  }

  SUBCASE("RangeSpansNegative") {
    deepnote::Range range(deepnote::RangeLow(-10), deepnote::RangeHigh(-5));
    CHECK(range.Length() == 5);
  }
}

