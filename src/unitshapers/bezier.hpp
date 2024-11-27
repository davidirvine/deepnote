#pragma once

#include "unitshaper.hpp"
     
class BezierUnitShaper : public UnitShaper 
{
public:
    BezierUnitShaper(const float y2, const float y3) :
        y2(y2), 
        y3(y3) 
    {}

    //
    //  TODO: optimize with functions from DaisySP/utility/dsp.h
    //
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
    float y1{0.0};  //  start point
    float y2{0.0};  //  control point 1
    float y3{0.0};  //  control point 2
    float y4{1.0};  //  end point
};