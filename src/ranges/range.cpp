#include "range.hpp"
#include "daisy_seed.h"

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

    float Range::random() const {
        return daisy::Random::GetFloat(low, high);
    }
}