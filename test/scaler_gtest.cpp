#include "scaler.hpp"
#include <gtest/gtest.h>

using namespace deepnotedrone;

TEST(ScalerTest, UnipolarInput) {
    auto input_range = Range(0, 1023);
    auto output_range = Range(0.0, 1.0);

    Scaler scaler = Scaler(input_range, output_range);

    EXPECT_EQ(scaler.scale(0.0), 0) << "Lower bounds failed";
    EXPECT_EQ(scaler.scale(1023.0), 1) << "Upper bounds failed";
}

TEST(ScalerTest, BipolarInput) {
    auto input_range = Range(-5.0, 5.0);
    auto output_range = Range(0.0, 1.0);

    Scaler scaler = Scaler(input_range, output_range);

    EXPECT_EQ(scaler.scale(-5.0), 0);
    EXPECT_EQ(scaler.scale(5.0), 1);
}

TEST(ScalerTest, BipolarOutput) {
    auto input_range = Range(0.0, 1.0);
    auto output_range = Range(-5.0, 5.0);

    Scaler scaler = Scaler(input_range, output_range);

    EXPECT_EQ(scaler.scale(0.5), 0);
    EXPECT_EQ(scaler.scale(0.0), -5);
}

