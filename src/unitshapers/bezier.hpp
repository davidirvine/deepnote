#pragma once

#include "util/namedtype.hpp"

namespace deepnote
{

    namespace nt
    {
        using ControlPoint1 = NamedType<float, struct ControlPoint1Tag>;
        using ControlPoint2 = NamedType<float, struct ControlPoint2Tag>;
    };

    /**
     * @brief Applies cubic Bezier curve shaping to unit input [0,1] -> [0,1]
     * 
     * Uses control points y2 and y3 with fixed endpoints y1=0, y4=1.
     * The curve equation: B(t) = (1-t)³*y1 + 3(1-t)²*t*y2 + 3(1-t)*t²*y3 + t³*y4
     * 
     * This allows for non-linear interpolation between start and end points,
     * enabling smooth acceleration/deceleration curves for audio parameter animation.
     * 
     * @param y2 First control point (influences curve shape near start)
     * @param y3 Second control point (influences curve shape near end)
     */
    struct BezierUnitShaper
    {
        BezierUnitShaper() = default;

        explicit BezierUnitShaper(const nt::ControlPoint1 y2, const nt::ControlPoint2 y3) : y2(y2.get()),
                                                                                            y3(y3.get())
        {
        }

        BezierUnitShaper(const BezierUnitShaper &other) = default;
        BezierUnitShaper &operator=(const BezierUnitShaper &other) = default;

        /**
         * @brief Apply Bezier curve transformation to input value
         * @param t Input value in range [0,1]
         * @return Shaped output value in range [0,1]
         */
        float operator()(const float t) const
        {
            float y =
                (1 - t) * (1 - t) * y1 +
                3 * (1 - t) * (1 - t) * t * y2 +
                3 * (1 - t) * t * t * y3 +
                t * t * t * y4;

            return y;
        }

    private:
        float y1{0.f}; //  start point
        float y2{0.f}; //  control point 1
        float y3{0.f}; //  control point 2
        float y4{1.f}; //  end point
    };

} // namespace deepnote
