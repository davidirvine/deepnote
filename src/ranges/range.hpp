/**
 * @file range.hpp
 * @brief Range constraint and validation utilities for the Deep Note synthesizer
 *
 * This file provides the Range class for defining and managing value ranges
 * with bounds checking and constraint capabilities. Used throughout the
 * synthesizer for parameter validation and value clamping.
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

#include "util/namedtype.hpp"

namespace deepnote
{

namespace nt
{
using RangeLow  = NamedType<float, struct RangeLowTag>;
using RangeHigh = NamedType<float, struct RangeHighTag>;
}; // namespace nt

struct Range
{
    Range()
        : low(0.f)
        , high(0.f)
    {
    }

    explicit Range(nt::RangeLow low, nt::RangeHigh high)
        : low(low.get() < high.get() ? low.get() : high.get())
        , high(high.get() > low.get() ? high.get() : low.get())
    {
    }

    Range(const Range &other)
        : low(other.low)
        , high(other.high)
    {
    }

    Range &operator=(const Range &other)
    {
        if(this != &other)
        {
            low  = other.low;
            high = other.high;
        }
        return *this;
    }

    nt::RangeLow get_low() const noexcept { return low; }

    nt::RangeHigh get_high() const noexcept { return high; }

    float length() const noexcept { return high.get() - low.get(); }

    bool contains(float value) const noexcept { return value >= low.get() && value <= high.get(); }

    float constrain(float value) const noexcept
    {
        if(value < low.get())
            return low.get();
        if(value > high.get())
            return high.get();
        return value;
    }

  private:
    nt::RangeLow  low;
    nt::RangeHigh high;
};

} // namespace deepnote
