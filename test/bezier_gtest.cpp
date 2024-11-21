#include "bezier.hpp"
#include <gtest/gtest.h>

using namespace deepnotedrone;

TEST(BezierTest, Endpoints) {
    BezierUnitShaper bezier(0.0, 1.0);
    EXPECT_EQ(bezier.shape(0.0), 0);
    EXPECT_EQ(bezier.shape(1.0), 1);
}