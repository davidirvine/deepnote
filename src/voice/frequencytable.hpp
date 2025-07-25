/**
 * @file frequencytable.hpp
 * @brief Frequency table implementation for voice management in the Deep Note synthesizer
 *
 * This file provides a templated frequency table class that stores and manages
 * frequency functions for multiple voices and indices. Used to organize and
 * access frequency data for the THX Deep Note effect.
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

#include "oscfrequency.hpp"
#include "ranges/range.hpp"
#include <array>
#include <functional>

namespace deepnote
{
namespace nt
{
using FrequencyTableIndex = NamedType<unsigned int, struct FrequencyTableIndexTag>;
using VoiceIndex          = NamedType<unsigned int, struct VoiceIndexTag>;
} // namespace nt

using FrequencyFunc = std::function<nt::OscillatorFrequency()>;

template <unsigned int TABLE_HEIGHT, unsigned int TABLE_WIDTH> struct FrequencyTable
{
    //  the table is a 2D array of functions that return a frequency
    using TableType = std::array<std::array<FrequencyFunc, TABLE_WIDTH>, TABLE_HEIGHT>;

    explicit FrequencyTable(const TableType &table)
        : freq_functions(table)
    {
    }

    FrequencyTable(const FrequencyTable<TABLE_HEIGHT, TABLE_WIDTH> &other) = default;
    FrequencyTable(FrequencyTable<TABLE_HEIGHT, TABLE_WIDTH> &&other)      = default;
    FrequencyTable<TABLE_HEIGHT, TABLE_WIDTH> &
    operator=(const FrequencyTable<TABLE_HEIGHT, TABLE_WIDTH> &other) = default;
    ~FrequencyTable()                                                 = default;

    nt::OscillatorFrequency get(const nt::FrequencyTableIndex table_index, const nt::VoiceIndex voice_index) const
    {
        // ensure that table_index and voice_index are within the bounds of the table
        // values will wrap around if they are out of bounds
        return freq_functions[table_index.get() % TABLE_HEIGHT][voice_index.get() % TABLE_WIDTH]();
    }

  private:
    TableType freq_functions;
};

} // namespace deepnote
