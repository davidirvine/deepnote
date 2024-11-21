#include "range.hpp"

namespace deepnotedrone {

    Range::Range(float low, float high) :
        low(low < high ? low : high),
        high(high > low ? high : low)
    {}

    float Range::length() const {
        return high - low;
    }

    bool Range::contains(float value) const {
        return value >= low && value <= high;
    }
}