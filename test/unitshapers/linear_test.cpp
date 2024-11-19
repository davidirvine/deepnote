#include "linear.hpp"
#include <gtest/gtest.h>

TEST(LinearTest, Endpoints) {
    LinearUnitShaper linear;
    EXPECT_EQ(linear(0.0), 0);
    EXPECT_EQ(linear(1.0), 1);
}