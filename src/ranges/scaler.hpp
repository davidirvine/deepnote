#ifndef __SCALER_HPP
#define __SCALER_HPP

#include "range.hpp"

namespace deepnotedrone {

    class Scaler {
        public:
            Scaler(Range input, Range output) : 
                input(input), 
                output(output) 
            {}

            Scaler(Range input) :
                input(input),
                output(Range(0.0, 1.0))
            {}
            
            float scale(float value) const {
                //
                //  normalize the input value to a range between 0.0 and 1.0
                //  then scale it to the output range
                //  then offset it to the output start
                //
                auto normalized_value = normalize(value);
                return normalized_value * output.length() + output.getLow();
            }

        private:

            float normalize(float value) const {
                //
                //  shift to a positive range from 0.0 to input.length()
                //  then divide by the input length to get a value between 0.0 and 1.0
                //
                return (value - input.getLow()) * 1.0 / input.length();
            }

            Range input;
            Range output;   
    };
}

#endif // __SCALER_HPP