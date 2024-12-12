#pragma once

#include "dsp.h"
#include "unitshaper.hpp"
    
namespace deepnote 
{
    
class BezierUnitShaper : public UnitShaper 
{
public:
    BezierUnitShaper() = default;

    BezierUnitShaper(const float y2, const float y3) :
        y2(y2), 
        y3(y3) 
    {}

    BezierUnitShaper(const BezierUnitShaper& other) :
        y2(other.y2), 
        y3(other.y3) 
    {}

    void operator=(const BezierUnitShaper& other) 
    {
        y2 = other.y2;
        y3 = other.y3;
    }

    float shape(const float t) 
    {
        float y = 
            (1-t) * (1-t) * y1 + 
            3 * (1-t) * (1-t) * t * y2 + 
            3 * (1-t) * t * t * y3 + 
            t * t * t * y4;

        return y;
    }

private:
    float y1{0.f};  //  start point
    float y2{0.f};  //  control point 1
    float y3{0.f};  //  control point 2
    float y4{1.f};  //  end point
};

} // namespace deepnote