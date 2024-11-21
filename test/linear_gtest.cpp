#include "linear.hpp"
#include <gtest/gtest.h>

using namespace deepnotedrone;

TEST(LinearTest, Endpoints) {
    LinearUnitShaper linear;
    EXPECT_EQ(linear.shape(0.0), 0);
    EXPECT_EQ(linear.shape(1.0), 1);
}