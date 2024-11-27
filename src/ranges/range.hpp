#pragma once

class Range 
{
public:
    Range() : 
        low(0.f), 
        high(0.f) 
    {};

    Range(float low, float high) :
        low(low < high ? low : high),
        high(high > low ? high : low)
    {};

    float getLow() const { return low; }
    float getHigh() const { return high;}
    
    float length() const { return high - low; };
    bool contains(float value) const { return value >= low && value <= high; };
private:
    float low;
    float high;
};
