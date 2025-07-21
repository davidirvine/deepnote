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

    FrequencyTable(const TableType &table) : freq_functions(table) { };
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
