#include "unitshapers/linear.hpp"
#include <doctest/doctest.h>

int get_index(float voct_voltage)
{
    return ((voct_voltage - (int)voct_voltage) / 0.083f) + 0.5f;
}

TEST_CASE("Math test")
{
    CHECK(get_index(0.00) == 0);
    CHECK(get_index(0.08) == 1);
    CHECK(get_index(0.16) == 2);
    CHECK(get_index(0.25) == 3);
    CHECK(get_index(0.33) == 4);
    CHECK(get_index(0.41) == 5);
    CHECK(get_index(0.5) == 6);
    CHECK(get_index(0.58) == 7);
    CHECK(get_index(0.66) == 8);
    CHECK(get_index(0.75) == 9);
    CHECK(get_index(0.83) == 10);
    CHECK(get_index(0.91) == 11);
    CHECK(get_index(1.0) == 0);
}

TEST_CASE("LinearUnitShaper::shape")
{
    deepnote::LinearUnitShaper linear;
    CHECK(linear(0.0) == 0);
    CHECK(linear(1.0) == 1);
}
