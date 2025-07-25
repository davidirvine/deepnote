/**
 * @file namedtype.hpp
 * @brief Type-safe wrapper utilities for the Deep Note synthesizer
 *
 * This file provides the NamedType template class for creating type-safe
 * wrappers around primitive types. Enhances code safety and readability
 * by preventing accidental parameter mismatches in function calls.
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

#include <utility>

namespace deepnote
{
//
//  Taken from: https://www.fluentcpp.com/2016/12/08/strong-types-for-strong-interfaces/
//
template <typename T, typename Parameter> struct NamedType
{
  public:
    explicit NamedType(const T &value)
        : value_(value)
    {
    }
    explicit NamedType(T &&value)
        : value_(std::move(value))
    {
    }
    T       &get() { return value_; }
    const T &get() const { return value_; }

    bool operator==(const NamedType<T, Parameter> &rhs) const { return value_ == rhs.value_; }

  private:
    T value_;
};

} // namespace deepnote
