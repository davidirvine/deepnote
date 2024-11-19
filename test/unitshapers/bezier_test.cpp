#include "bezier.hpp"
#include <gtest/gtest.h>

TEST(BezierTest, Endpoints) {
    BezierUnitShaper bezier(0.0, 1.0);
    EXPECT_EQ(bezier(0.0), 0);
    EXPECT_EQ(bezier(1.0), 1);
}