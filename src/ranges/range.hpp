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
        Range() : low(0.f),
                  high(0.f)
        {
        }

        Range(nt::RangeLow low, nt::RangeHigh high) : low(low.get() < high.get() ? low.get() : high.get()),
                                                      high(high.get() > low.get() ? high.get() : low.get())
        {
        }

        Range(const Range &other) : low(other.low),
                                    high(other.high)
        {
        }

        Range &operator=(const Range &other)
        {
            if (this != &other)
            {
                low = other.low;
                high = other.high;
            }
            return *this;
        }

        nt::RangeLow get_low() const
        {
            return low;
        }

        nt::RangeHigh get_high() const
        {
            return high;
        }

        float length() const
        {
            return high.get() - low.get();
        }

        bool contains(float value) const
        {
            return value >= low.get() && value <= high.get();
        }

        float constrain(float value) const
        {
            if (value < low.get())
                return low.get();
            if (value > high.get())
                return high.get();
            return value;
        }

    private:
        nt::RangeLow low;
        nt::RangeHigh high;
    };

} // namespace deepnote
