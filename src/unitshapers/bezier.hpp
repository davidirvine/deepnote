#ifndef __BEZIER_UNIT_SHAPER_HPP
#define __BEZIER_UNIT_SHAPER_HPP

#include "unitshaper.hpp"

namespace deepnotedrone {
        
    class BezierUnitShaper : public UnitShaper {
        public:
            BezierUnitShaper(float y2, float y3) :
                y2(y2), y3(y3) {}

            float operator()(float t) {
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
}
#endif // __BEZIER_UNIT_SHAPER_HPP