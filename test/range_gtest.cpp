#include "range.hpp"
#include <gtest/gtest.h>

using namespace deepnotedrone;

TEST(RangeTest, RangeLength) {
  Range range(0, 10);
  EXPECT_EQ(range.length(), 10);
}

TEST(RangeTEst, RangeSpansZero) {
  Range range(-10, 10);
  EXPECT_EQ(range.length(), 20);
}

TEST(RangeTEst, RangeProvidedBackwards) {
  Range range(10, -10);
  EXPECT_EQ(range.length(), 20);
}

TEST(RangeTEst, RangeNegative) {
  Range range(-10, -5);
  EXPECT_EQ(range.length(), 5);
}

