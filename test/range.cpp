#include "ranges/range.hpp"
#include <doctest/doctest.h>

namespace nt = deepnote::nt;

TEST_CASE("Range")
{
  SUBCASE("Range::Range")
  {
    deepnote::Range range(nt::RangeLow(0), nt::RangeHigh(10));
    CHECK(range.get_low().get() == 0);
    CHECK(range.get_high().get() == 10);
  }

  SUBCASE("RangeSpansPositive")
  {
    deepnote::Range range(nt::RangeLow(0), nt::RangeHigh(10));
    CHECK(range.length() == 10);
  }

  SUBCASE("RangeSpansZero")
  {
    deepnote::Range range(nt::RangeLow(-10), nt::RangeHigh(10));
    CHECK(range.length() == 20);
  }

  SUBCASE("RangeSpansNegative")
  {
    deepnote::Range range(nt::RangeLow(-10), nt::RangeHigh(-5));
    CHECK(range.length() == 5);
  }
}
