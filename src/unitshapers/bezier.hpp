#pragma once

#include "util/namedtype.hpp"

namespace deepnote
{

    namespace nt
    {
        using ControlPoint1 = NamedType<float, struct ControlPoint1Tag>;
        using ControlPoint2 = NamedType<float, struct ControlPoint2Tag>;
    };

    class BezierUnitShaper
    {
    public:
        BezierUnitShaper() = default;

        BezierUnitShaper(const nt::ControlPoint1 y2, const nt::ControlPoint2 y3) : y2(y2.get()),
                                                                                   y3(y3.get())
        {
        }

        BezierUnitShaper(const BezierUnitShaper &other) : y2(other.y2),
                                                          y3(other.y3)
        {
        }

        BezierUnitShaper &operator=(const BezierUnitShaper &other)
        {
            if (this != &other)
            {
                y2 = other.y2;
                y3 = other.y3;
            }
            return *this;
        }

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
