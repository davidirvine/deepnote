#pragma once

namespace deepnote 
{

class Range 
{
public:
    Range() : 
        low(0.f), 
        high(0.f) 
    {}

    Range(float low, float high) :
        low(low < high ? low : high),
        high(high > low ? high : low)
    {}

    Range(const Range& other) :
        low(other.low),
        high(other.high)
    {}

    void operator=(const Range& other) 
    {
        low = other.low;
        high = other.high;
    }
    
    float getLow() const { return low; }
    float getHigh() const { return high;}
    
    float length() const { return high - low; }
    bool contains(float value) const { return value >= low && value <= high; }

    float constrain(float value) const 
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
