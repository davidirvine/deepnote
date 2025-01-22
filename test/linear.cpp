#include "unitshapers/linear.hpp"
#include <doctest/doctest.h>


int getIndex(float voct_voltage) {
    const float int_voct = (int)voct_voltage;
    const float remainder = voct_voltage - int_voct;
    const float oftwelve = (remainder / 0.083f);
    //const int index = (int)oftwelve;
    
    const int index = ((voct_voltage - (int)voct_voltage) / 0.083f) + 0.5f;
    return index;
}

TEST_CASE("Math test") {
    CHECK(getIndex(0.00) == 0);
    CHECK(getIndex(0.08) == 1);
    CHECK(getIndex(0.16) == 2);
    CHECK(getIndex(0.25) == 3);
    CHECK(getIndex(0.33) == 4);
    CHECK(getIndex(0.41) == 5);
    CHECK(getIndex(0.5) == 6);
    CHECK(getIndex(0.58) == 7);
    CHECK(getIndex(0.66) == 8);
    CHECK(getIndex(0.75) == 9);
    CHECK(getIndex(0.83) == 10);
    CHECK(getIndex(0.91) == 11);
    CHECK(getIndex(1.0) == 0);
}


TEST_CASE("LinearUnitShaper::shape") {
    deepnote::LinearUnitShaper linear;
    CHECK(linear(0.0) == 0);
    CHECK(linear(1.0) == 1);
}