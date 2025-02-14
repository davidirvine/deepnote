//
// The THX logo theme consists of 30 voices over seven measures, starting
// in a narrow range, 200 to 400 Hz, and slowly diverging to preselected
// pitches encompassing three octaves. The 30 voices begin at pitches between
// 200 Hz and 400 Hz and arrive at` pre-selected pitches spanning three
// octaves by the fourth measure. The highest pitch is slightly detuned while
// there is double the number of voices of the lowest two pitches.
//

#include "daisy_seed.h"
#include "voice/frequencytable.hpp"
#include "voice/deepnotevoice.hpp"
#include "ranges/range.hpp"
#include "daisy_core.h"
#include <array>
#include <atomic>
#include <functional>
#include <cmath>

namespace nt = deepnote::nt;

// Voices
const int NUM_OSC_DUO = 2;
const int NUM_DUO_VOICES = 5;

const int NUM_OSC_TRIO = 3;
const int NUM_TRIO_VOICES = 4;

using DuoVoiceType = deepnote::DeepnoteVoice<NUM_OSC_DUO>;
std::array<DuoVoiceType, NUM_DUO_VOICES> duo_voices;

using TrioVoiceType = deepnote::DeepnoteVoice<NUM_OSC_TRIO>;
std::array<TrioVoiceType, NUM_TRIO_VOICES> trio_voices;

const int FREQ_TABLE_WIDTH = NUM_TRIO_VOICES + NUM_DUO_VOICES;
const int FREQ_TABLE_HEIGHT = 13;

// Generate a random frequency with in a range of frequencies
// This function is stored in the the first row of the frequency table allowing
// the drone to start with a new random "chord" each time it is started.
deepnote::FrequencyFunc random_start_freq = []()
{
	const auto low = nt::RangeLow(200.f);
	const auto high = nt::RangeHigh(400.f);
	return nt::OscillatorFrequency(daisy::Random::GetFloat(low.get(), high.get()));
};

// Return the fucntion that returns a fixed frequency
deepnote::FrequencyFunc freq(const float f)
{
	return deepnote::FrequencyFunc([f]()
								  { return nt::OscillatorFrequency(f); });
};

// The rows contain the frequencies for the target "chord" to be played by the drone.
// There is one frequency per voice in the drone in each column.
// The first row will contain a random start "chord".
// Subsequent rows will contain the the chord rooted at all 12 notes in the
// chromatic scale.
//
// 1 = C, 2 = C#, 3 = D, 4 = D#, 5 = E, 6 = F, 7 = F#, 8 = G, 9 = G#, 10 = A, 11 = A#, 12 = B
//
// See tools/freqtable-builder.py for the generation of the values in the table
deepnote::FrequencyTable<FREQ_TABLE_HEIGHT, FREQ_TABLE_WIDTH> target_freq_table({{
	{random_start_freq, random_start_freq, random_start_freq, random_start_freq, random_start_freq, random_start_freq, random_start_freq, random_start_freq, random_start_freq},
	{freq(1244.51f), freq(1046.50f), freq(587.33f), freq(523.25f), freq(392.00f), freq(130.81f), freq(98.00f), freq(65.41f), freq(32.70f)},
	{freq(1318.51f), freq(1108.73f), freq(622.25f), freq(554.37f), freq(415.30f), freq(138.59f), freq(103.83f), freq(69.30f), freq(34.65f)},
	{freq(1396.91f), freq(1174.66f), freq(659.26f), freq(587.33f), freq(440.00f), freq(146.83f), freq(110.00f), freq(73.42f), freq(36.71f)},
	{freq(1479.98f), freq(1244.51f), freq(698.46f), freq(622.25f), freq(466.16f), freq(155.56f), freq(116.54f), freq(77.78f), freq(38.89f)},
	{freq(1567.98f), freq(1318.51f), freq(739.99f), freq(659.26f), freq(493.88f), freq(164.81f), freq(123.47f), freq(82.41f), freq(41.20f)},
	{freq(1661.22f), freq(1396.91f), freq(783.99f), freq(698.46f), freq(523.25f), freq(174.61f), freq(130.81f), freq(87.31f), freq(43.65f)},
	{freq(1760.00f), freq(1479.98f), freq(830.61f), freq(739.99f), freq(554.37f), freq(185.00f), freq(138.59f), freq(92.50f), freq(46.25f)},
	{freq(1864.66f), freq(1567.98f), freq(880.00f), freq(783.99f), freq(587.33f), freq(196.00f), freq(146.83f), freq(98.00f), freq(49.00f)},
	{freq(1975.53f), freq(1661.22f), freq(932.33f), freq(830.61f), freq(622.25f), freq(207.65f), freq(155.56f), freq(103.83f), freq(51.91f)},
	{freq(2093.00f), freq(1760.00f), freq(987.77f), freq(880.00f), freq(659.26f), freq(220.00f), freq(164.81f), freq(110.00f), freq(55.00f)},
	{freq(2217.46f), freq(1864.66f), freq(1046.50f), freq(932.33f), freq(698.46f), freq(233.08f), freq(174.61f), freq(116.54f), freq(58.27f)},
	{freq(2349.32f), freq(1975.53f), freq(1108.73f), freq(987.77f), freq(739.99f), freq(246.94f), freq(185.00f), freq(123.47f), freq(61.74f)}}});

// Generate a random animation frequency
nt::OscillatorFrequency random_animation_freq()
{
	const auto low = nt::RangeLow(0.5f);
	const auto high = nt::RangeHigh(1.5f);
	return nt::OscillatorFrequency(daisy::Random::GetFloat(low.get(), high.get()));
}

// ADC channels used for external controls
enum AdcChannelId
{
	DETUNE = 0, // oscillator detune
	TARGET,		// root note of the target chord
	VOLUME,		// volume
	NUM_ADC_CHANNEL
};

// State variables used to communicate state between the
// audio callback and the main loop
std::atomic<float> value_detune{2.5f};
std::atomic<float> value_volume{0.f};
std::atomic<int> value_target{0};
std::atomic<bool> flag_toggle_changed{false};
std::atomic<bool> flag_target_changed{false};

bool floatEqual(float a, float b, float epsilon = 0.0001f)
{
	return std::fabs(a - b) < epsilon;
}

void AudioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t buffer_size)
{
	const auto detune = nt::DetuneHz(value_detune);
	const auto animation_multiplier = nt::AnimationMultiplier(1.f);
	const auto cp1 = nt::ControlPoint1(0.08f);
	const auto cp2 = nt::ControlPoint2(0.5f);
	const auto trace_functor = deepnote::NoopTrace();

	for (size_t bufferIndex = 0; bufferIndex < buffer_size; bufferIndex++)
	{
		auto output{0.f};
		auto index{0u};
		for (auto &voice : trio_voices)
		{
			voice.set_detune(detune);
			if (flag_target_changed)
			{
				voice.set_target_frequency(target_freq_table.get(nt::FrequencyTableIndex(value_target), nt::VoiceIndex(index++)));
			}
			if (flag_toggle_changed)
			{
				voice.set_start_frequency(target_freq_table.get(nt::FrequencyTableIndex(0), nt::VoiceIndex(index++)));
			}
			output += voice.process(animation_multiplier, cp1, cp2, trace_functor);
		}

		for (auto &voice : duo_voices)
		{
			voice.set_detune(detune);
			if (flag_target_changed)
			{
				voice.set_target_frequency(target_freq_table.get(nt::FrequencyTableIndex(value_target), nt::VoiceIndex(index++)));
			}
			if (flag_toggle_changed)
			{
				voice.set_start_frequency(target_freq_table.get(nt::FrequencyTableIndex(0), nt::VoiceIndex(index++)));
			}
			output += voice.process(animation_multiplier, cp1, cp2, trace_functor);
		}

		output *= value_volume;

		out[0][bufferIndex] = output;
		out[1][bufferIndex] = output;
	}
}

int main(void)
{
	daisy::DaisySeed hw;

	hw.Init();
	hw.SetAudioBlockSize(4);
	hw.SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate::SAI_48KHZ);
	auto sample_rate = hw.AudioSampleRate();

	// Setup logging
	//
	// HEADS UP! StartLog will block until a serial terminal is connected
	hw.StartLog(true);
	hw.PrintLine("deepnotedrone starting...");

	// Configure ADC channels
	daisy::AdcChannelConfig adcChannels[NUM_ADC_CHANNEL];
	adcChannels[DETUNE].InitSingle(daisy::seed::A2);
	adcChannels[TARGET].InitSingle(daisy::seed::A1);
	adcChannels[VOLUME].InitSingle(daisy::seed::A0);
	hw.adc.Init(adcChannels, NUM_ADC_CHANNEL);

	// toggle switch
	daisy::Switch toggle;
	toggle.Init(
		daisy::seed::D18,
		0.f,
		daisy::Switch::Type::TYPE_TOGGLE,
		daisy::Switch::Polarity::POLARITY_NORMAL,
		daisy::GPIO::Pull::PULLUP);

	// indexes used for target_freq_table lookup
	auto voice_index{0};
	const auto start_table_index{0};

	// initialize the voices
	for (auto &voice : trio_voices)
	{
		voice.init(
			target_freq_table.get(nt::FrequencyTableIndex(start_table_index), nt::VoiceIndex(voice_index++)),
			nt::SampleRate(sample_rate),
			random_animation_freq());
	}

	for (auto &voice : duo_voices)
	{
		voice.init(
			target_freq_table.get(nt::FrequencyTableIndex(start_table_index), nt::VoiceIndex(voice_index++)),
			nt::SampleRate(sample_rate),
			random_animation_freq());
	}

	//	Start ADC and real-time audio processing
	hw.adc.Start();
	hw.StartAudio(AudioCallback);

	//	Loop forever performing non real-time tasks
	bool led_state{true};

	while (1)
	{
		static bool toggle_state{false};

		// blink the LED as a sign of life
		hw.SetLed(led_state);
		led_state = !led_state;

		// detune
		const deepnote::Range DETUNE_RANGE{nt::RangeLow(0.f), nt::RangeHigh(5.f)};
		const float detune = daisysp::fmap(
			hw.adc.GetFloat(DETUNE),
			DETUNE_RANGE.get_low().get(),
			DETUNE_RANGE.get_high().get(),
			daisysp::Mapping::LINEAR);

		if (!floatEqual(value_detune, detune, 0.0005f))
		{
			value_detune = detune;
			hw.PrintLine("Detune " FLT_FMT3, FLT_VAR3(value_detune));
		}

		// value_target will contain integer values between 1 and 12
		// representing the index of the target chord in the frequency table
		const int target = static_cast<int>(std::floor(std::floor(hw.adc.GetFloat(TARGET) * (FREQ_TABLE_HEIGHT - 1.f))) + 1);
		flag_target_changed = (target != value_target);

		if (flag_target_changed)
		{
			const int na_target = value_target;
			hw.PrintLine("Target changed from %d to %d", target, na_target);
			value_target = target;
		}

		// volume
		const float volume = daisysp::fmap(
			hw.adc.GetFloat(VOLUME),
			0.f,
			1.f,
			daisysp::Mapping::LINEAR);

		if (!floatEqual(value_volume, volume, 0.0005f))
		{
			hw.PrintLine("Volume " FLT_FMT3, FLT_VAR3(volume));
			value_volume = volume;
		}

		// toggle switch
		const auto raw_toggle_state = toggle.RawState();
		flag_toggle_changed = (toggle_state != raw_toggle_state);
		toggle_state = raw_toggle_state;
		if (flag_toggle_changed)
		{
			hw.PrintLine("Toggled");
		}

		hw.DelayMs(200);
	}
}
