#pragma once

#include "range.hpp"
#include "util/namedtype.hpp"

namespace deepnote
{

    namespace nt
    {
        using InputRange = NamedType<Range, struct InputRangeTag>;
        using OutputRange = NamedType<Range, struct OutputRangeTag>;
    };

    struct Scaler
    {
        Scaler() : input(Range(nt::RangeLow(0.0), nt::RangeHigh(1.0))),
                   output(Range(nt::RangeLow(0.0), nt::RangeHigh(1.0)))
        {
        }

        explicit Scaler(const nt::InputRange input, const nt::OutputRange output) : input(input.get()),
                                                                                    output(output.get())
        {
        }

        explicit Scaler(const nt::InputRange input) : Scaler(input, nt::OutputRange(Range(nt::RangeLow(0.0), nt::RangeHigh(1.0))))
        {
        }

        Scaler(const Scaler &other) = default;
        Scaler &operator=(const Scaler &other) = default;

        float operator()(const float value) const
        {
            //
            //  normalize the input value to a range between 0.0 and 1.0
            //  then scale it to the output range
            //  then offset it to the output start
            //
            return (normalize(value) * output.length()) + output.get_low().get();
        }

    private:
        float normalize(const float value) const
        {
            return (value - input.get_low().get()) / input.length();
        }

        Range input;
        Range output;
    };

} // namespace deepnote
