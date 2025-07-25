/**
 * @file scaler.hpp
 * @brief Value scaling and mapping utilities for the Deep Note synthesizer
 *
 * This file provides the Scaler class for mapping values between different
 * ranges. Essential for converting normalized control values to frequency
 * ranges and other parameter mappings in the synthesizer.
 *
 * @author David Irvine
 * @date 2025
 * @copyright MIT License
 *
 * Copyright (c) 2025 David Irvine
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "range.hpp"
#include "util/namedtype.hpp"

namespace deepnote
{

namespace nt
{
using InputRange  = NamedType<Range, struct InputRangeTag>;
using OutputRange = NamedType<Range, struct OutputRangeTag>;
}; // namespace nt

struct Scaler
{
    Scaler()
        : input(Range(nt::RangeLow(0.0), nt::RangeHigh(1.0)))
        , output(Range(nt::RangeLow(0.0), nt::RangeHigh(1.0)))
    {
    }

    explicit Scaler(const nt::InputRange input, const nt::OutputRange output)
        : input(input.get())
        , output(output.get())
    {
    }

    explicit Scaler(const nt::InputRange input)
        : Scaler(input, nt::OutputRange(Range(nt::RangeLow(0.0), nt::RangeHigh(1.0))))
    {
    }

    Scaler(const Scaler &other)            = default;
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
    float normalize(const float value) const { return (value - input.get_low().get()) / input.length(); }

    Range input;
    Range output;
};

} // namespace deepnote
