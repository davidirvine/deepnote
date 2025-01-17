#pragma once

#include "util/namedtype.hpp"

namespace deepnote 
{

namespace nt
{
    using RangeLow = NamedType<float, struct RangeLowTag>;
    using RangeHigh = NamedType<float, struct RangeHighTag>;
};

class Range 
{
public:
    Range() : 
        low(0.f), 
        high(0.f) 
    {}

    Range(nt::RangeLow low, nt::RangeHigh high) :
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
    
    nt::RangeLow GetLow() const 
    { 
        return low; 
    }

    nt::RangeHigh GetHigh() const 
    { 
        return high;
    }
    
    float Length() const 
    { 
        return high.get() - low.get(); 
    }

    bool Contains(float value) const 
    {
        return value >= low.get() && value <= high.get(); 
    }

    float Constrain(float value) const 
    {
        if (value < low.get()) return low.get();
        if (value > high.get()) return high.get();
        return value;
    }
    
private:
    nt::RangeLow low;
    nt::RangeHigh high;
};

} // namespace deepnote
