#pragma once

#include "range.hpp"
#include "util/namedtype.hpp"

namespace deepnote 
{

using InputRange = NamedType<Range, struct InputRangeTag>;
using OutputRange = NamedType<Range, struct OutputRangeTag>;

class Scaler 
{
public:
    Scaler() : 
        input(Range(RangeLow(0.0), RangeHigh(1.0))), 
        output(Range(RangeLow(0.0), RangeHigh(1.0))) 
    {}

    Scaler(const InputRange input, const OutputRange output) : 
        input(input.get()), 
        output(output.get()) 
    {}

    Scaler(const InputRange input) : Scaler(input, OutputRange(Range(RangeLow(0.0), RangeHigh(1.0)))) 
    {}
    
    Scaler(const Scaler& other) : 
        input(other.input), 
        output(other.output) 
    {}
    
    Scaler& operator=(const Scaler& other) 
    {
        if (this != &other)
        {
            input = other.input;
            output = other.output;
        }
        return *this;
    }

    float operator()(const float value) const 
    {
        //
        //  normalize the input value to a range between 0.0 and 1.0
        //  then scale it to the output range
        //  then offset it to the output start
        //
        return (Normalize(value) * output.Length()) + output.GetLow();
    }

private:

    float Normalize(const float value) const 
    {
        return (value - input.GetLow()) / input.Length();
    }

    Range input;
    Range output;   
};

} // namespace deepnote