#pragma once

#include "range.hpp"

namespace deepnote 
{

class Scaler 
{
public:
    Scaler() : 
        input(Range(0.0, 1.0)), 
        output(Range(0.0, 1.0)) 
    {}

    Scaler(const Range input, const Range output) : 
        input(input), 
        output(output) 
    {}

    Scaler(const Range input) : Scaler(input, Range(0.0, 1.0)) 
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

    float Scale(const float value) const 
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