#ifndef __SCALER_HPP
#define __SCALER_HPP

#include "range.hpp"

namespace deepnotedrone {

    class Scaler {
        public:
            Scaler(const Range input, const Range output) : 
                input(input), 
                output(output) 
            {}

            Scaler(const Range input) :
                input(input),
                output(Range(0.0, 1.0))
            {}
            
            float scale(const float value) const {
                //
                //  normalize the input value to a range between 0.0 and 1.0
                //  then scale it to the output range
                //  then offset it to the output start
                //
                return (normalize(value) * output.length()) + output.getLow();
            }

        private:

            float normalize(const float value) const {
                return (value - input.getLow()) / input.length();
            }

            Range input;
            Range output;   
    };
}

#endif // __SCALER_HPP