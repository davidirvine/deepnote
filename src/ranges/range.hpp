#pragma once

#include "util/namedtype.hpp"

namespace deepnote 
{

using RangeLow = NamedType<float, struct RangeLowTag>;
using RangeHigh = NamedType<float, struct RangeHighTag>;


class Range 
{
public:
    Range() : 
        low(0.f), 
        high(0.f) 
    {}

    Range(RangeLow low, RangeHigh high) :
        low(low.get() < high.get() ? low.get() : high.get()),
        high(high.get() > low.get() ? high.get() : low.get())
    {}

    Range(const Range& other) :
        low(other.low),
        high(other.high)
    {}

    Range& operator=(const Range& other) 
    {
        if (this != &other)
        {
            low = other.low;
            high = other.high;
        }
        return *this;
    }
    
    float GetLow() const 
    { 
        return low; 
    }

    float GetHigh() const 
    { 
        return high;
    }
    
    float Length() const 
    { 
        return high - low; 
    }

    bool Contains(float value) const 
    {
        return value >= low && value <= high; 
    }

    float Constrain(float value) const 
    {
        if (value < low) return low;
        if (value > high) return high;
        return value;
    }
    
private:
    float low;
    float high;
};

} // namespace deepnote
