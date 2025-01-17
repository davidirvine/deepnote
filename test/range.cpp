#include "ranges/range.hpp"
#include <doctest/doctest.h>

namespace types = deepnote::nt;

TEST_CASE("Range") {
  SUBCASE("Range::Range") {
    deepnote::Range range(types::RangeLow(0), types::RangeHigh(10));
    CHECK(range.GetLow().get() == 0);
    CHECK(range.GetHigh().get() == 10);
  }

  SUBCASE("RangeSpansPositive") {
    deepnote::Range range(types::RangeLow(0), types::RangeHigh(10));
    CHECK(range.Length() == 10);
  }

  SUBCASE("RangeSpansZero") {
    deepnote::Range range(types::RangeLow(-10), types::RangeHigh(10));
    CHECK(range.Length() == 20);
  }

  SUBCASE("RangeSpansNegative") {
    deepnote::Range range(types::RangeLow(-10), types::RangeHigh(-5));
    CHECK(range.Length() == 5);
  }
}

