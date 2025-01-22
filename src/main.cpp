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
#include "Filters/moogladder.h"
#include "per/rng.h"
#include "util/namedtype.hpp"
#include "daisy_core.h"

namespace types = deepnote::nt;

//	deepnotedrone::LibDaisyRandom is a random number generator
//	that uses the libDaisy random number generator.
struct LibDaisyRandom
{
	float operator()(float low, float high) const
	{
		return daisy::Random::GetFloat(low, high);
	}
};

//
//	Voices
//
using DuoVoiceType = deepnote::DeepnoteVoice<2>;
using TrioVoiceType = deepnote::DeepnoteVoice<3>;
const auto START_FREQ_RANGE = types::OscillatorFrequencyRange(deepnote::Range(
	types::RangeLow(200.f),
	types::RangeHigh(400.f)));
const deepnote::Range ANIMATION_RATE_RANGE(types::RangeLow(0.05f), types::RangeHigh(1.5f));

TrioVoiceType trio_voices[deepnote::NUM_TRIO_VOICES];
DuoVoiceType duo_voices[deepnote::NUM_DUO_VOICES];
deepnote::FrequencyTable voice_frequencies;

enum AdcChannelId
{
	DETUNE = 0,
	FILTER_CUTOFF,
	VOLUME,
	NUM_ADC_CHANNEL
};

//	Values for the knobs and switches
auto value_detune{2.5f};
auto value_filter_cutoff{0.f};
auto value_volume{0.f};
bool value_travel_selector{false};

const deepnote::Range animationRateRange{types::RangeLow(0.05f), types::RangeHigh(1.5f)};
daisysp::MoogLadder filter;

void AudioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t buffer_size)
{
	filter.SetFreq(value_filter_cutoff);

	//	TODO: read the index and detune from potentiometers
	const auto index_changed = voice_frequencies.set_current_index(types::FrequencyTableIndex(1));
	const auto detune = types::DetuneHz(value_detune);
	const auto animation_multiplier = types::AnimationMultiplier(1.f);
	const auto cp1 = types::ControlPoint1(0.08f);
	const auto cp2 = types::ControlPoint2(0.5f);
	const auto trace_functor = deepnote::NoopTrace();

	for (size_t bufferIndex = 0; bufferIndex < buffer_size; bufferIndex++)
	{
		auto output{0.f};
		auto index{0};
		for (auto &voice : trio_voices)
		{
			voice.compute_detune(detune);
			if (index_changed)
			{
				voice.set_target_frequency(voice_frequencies.get_frequency(types::VoiceIndex(index++)));
			}
			output += voice.process(animation_multiplier, cp1, cp2, trace_functor);
		}

		for (auto &voice : duo_voices)
		{
			voice.compute_detune(detune);
			if (index_changed)
			{
				voice.set_target_frequency(voice_frequencies.get_frequency(types::VoiceIndex(index++)));
			}
			output += voice.process(animation_multiplier, cp1, cp2, trace_functor);
		}

		output = filter.Process(output);

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

	//
	// Setup logging
	//
	// HEADS UP! StartLog will block until a serial terminal is connected
	//
	hw.StartLog(true);
	hw.PrintLine("deepnotedrone starting...");

	//
	//	Configure IO for external controls
	//
	//	Configure ADC channels
	daisy::AdcChannelConfig adcChannels[NUM_ADC_CHANNEL];
	//	oscillator detune channel
	adcChannels[DETUNE].InitSingle(daisy::seed::A2);
	//	filter cutoff channel
	adcChannels[FILTER_CUTOFF].InitSingle(daisy::seed::A1);
	//	volume channel
	adcChannels[VOLUME].InitSingle(daisy::seed::A0);
	//	Initialize the ADC
	hw.adc.Init(adcChannels, NUM_ADC_CHANNEL);

	//	waveform selector switch
	daisy::Switch travel_selector;
	travel_selector.Init(
		daisy::seed::D18,
		0.f,
		daisy::Switch::Type::TYPE_TOGGLE,
		daisy::Switch::Polarity::POLARITY_NORMAL,
		daisy::GPIO::Pull::PULLUP);

	const LibDaisyRandom random;
	auto index{0};
	voice_frequencies.initialize(START_FREQ_RANGE, random);

	//
	//	initialize the voices
	//
	for (auto &voice : trio_voices)
	{
		voice.init(
			voice_frequencies.get_frequency(types::VoiceIndex(index++)),
			types::SampleRate(sample_rate),
			types::OscillatorFrequency(random(ANIMATION_RATE_RANGE.get_low().get(), ANIMATION_RATE_RANGE.get_high().get())),
			random);
	}

	for (auto &voice : duo_voices)
	{
		voice.init(
			voice_frequencies.get_frequency(types::VoiceIndex(index++)),
			types::SampleRate(sample_rate),
			types::OscillatorFrequency(random(ANIMATION_RATE_RANGE.get_low().get(), ANIMATION_RATE_RANGE.get_high().get())),
			random);
	}

	filter.Init(sample_rate);
	filter.SetRes(0.0f);

	//	Start ADC and real-time audio processing
	hw.adc.Start();
	hw.StartAudio(AudioCallback);

	value_travel_selector = travel_selector.RawState();

	//	Loop forever performing non real-time tasks
	while (1)
	{
		const deepnote::Range OSC_FREQ_RANGE{types::RangeLow(10.f), types::RangeHigh(2000.f)};
		const deepnote::Range FILTER_FREQ_RANGE{types::RangeLow(500.f), types::RangeHigh(15000.f)};
		const deepnote::Range DETUNE_RANGE{types::RangeLow(0.f), types::RangeHigh(5.f)};

		value_detune = daisysp::fmap(
			hw.adc.GetFloat(DETUNE),
			DETUNE_RANGE.get_low().get(),
			DETUNE_RANGE.get_high().get(),
			daisysp::Mapping::LINEAR);

		value_filter_cutoff = daisysp::fmap(
			hw.adc.GetFloat(FILTER_CUTOFF),
			FILTER_FREQ_RANGE.get_low().get(),
			FILTER_FREQ_RANGE.get_high().get(),
			daisysp::Mapping::LINEAR);

		value_volume = daisysp::fmap(
			hw.adc.GetFloat(VOLUME),
			0.f,
			1.f,
			daisysp::Mapping::LINEAR);

		hw.PrintLine("Detune " FLT_FMT3, FLT_VAR3(value_detune));
		hw.PrintLine("Cutoff " FLT_FMT3, FLT_VAR3(value_filter_cutoff));
		hw.PrintLine("Volume " FLT_FMT3, FLT_VAR3(value_volume));
		hw.PrintLine("Travel %d", value_travel_selector);

		hw.DelayMs(200);
	}
}
