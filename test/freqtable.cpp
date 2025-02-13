#include "voice/frequencytable.hpp"
#include <doctest/doctest.h>
#include <functional>

namespace nt = deepnote::nt;


deepnote::FreqencyFunc fn(const float f) {
	return deepnote::FreqencyFunc([f]() {
		return nt::OscillatorFrequency(f);
	});
};

TEST_CASE("FrequencyTable")
{
  deepnote::FrequencyTable<2, 2> table({{
    {fn(1.f), fn(2.f)},
    {fn(3.f), fn(4.f)}
  }});

  CHECK(table.get(nt::FrequencyTableIndex(0), nt::VoiceIndex(0)) == nt::OscillatorFrequency(1.f));
  CHECK(table.get(nt::FrequencyTableIndex(0), nt::VoiceIndex(1)) == nt::OscillatorFrequency(2.f));
  CHECK(table.get(nt::FrequencyTableIndex(1), nt::VoiceIndex(0)) == nt::OscillatorFrequency(3.f));
  CHECK(table.get(nt::FrequencyTableIndex(1), nt::VoiceIndex(1)) == nt::OscillatorFrequency(4.f));

  //  (5, 5) should wrap to table[5 % 2][5 % 2] = table[1][1] = 4.f 
  CHECK(table.get(nt::FrequencyTableIndex(5), nt::VoiceIndex(5)) == nt::OscillatorFrequency(4.f));
}
