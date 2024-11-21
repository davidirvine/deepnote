#ifndef __RANGE_HPP
#define __RANGE_HPP

namespace deepnotedrone {

class Range {
    public:
        float low;
        float high;

        Range() = default;
        Range(float low, float high);

        float getLow() const { return low; }
        float getHigh() const { return high;}
        
        float length() const;
        bool contains(float value) const;
};

}

#endif // __RANGE_HPP