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
		types::RangeHigh(400.f)
));
const deepnote::Range ANIMATION_RATE_RANGE(types::RangeLow(0.05f), types::RangeHigh(1.5f));

TrioVoiceType trioVoices[deepnote::NUM_TRIO_VOICES];
DuoVoiceType duoVoices[deepnote::NUM_DUO_VOICES];
deepnote::FrequencyTable voiceFrequencies;

enum AdcChannelId 
{
	DETUNE = 0,
	FILTER_CUTOFF,
	VOLUME,
	NUM_ADC_CHANNEL
};

//	Values for the knobs and switches
auto valueDetune{2.5f};
auto valueFilterCutoff{0.f};
auto valueVolume{0.f};
bool valueTravelSelector{false};

const deepnote::Range animationRateRange{types::RangeLow(0.05f), types::RangeHigh(1.5f)};
daisysp::MoogLadder filter;

void AudioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t bufferSize)
{
	filter.SetFreq(valueFilterCutoff);

	//	TODO: read the index and detune from potentiometers
	const auto indexChanged = voiceFrequencies.setCurrentIndex(types::FrequencyTableIndex(1));
	const auto detune = types::DetuneHz(valueDetune);
	const auto animationMultiplier = types::AnimationMultiplier(1.f);
	const auto cp1 = types::ControlPoint1(0.08f);
	const auto cp2 = types::ControlPoint2(0.5f);
	const auto traceFunctor = deepnote::NoopTrace();

	for (size_t bufferIndex = 0; bufferIndex < bufferSize; bufferIndex++) {
		auto output{0.f};
		auto index{0};
		for (auto& voice : trioVoices) {
			voice.computeDetune(detune);
			if (indexChanged)
			{
				voice.SetTargetFrequency(voiceFrequencies.getFrequency(types::VoiceIndex(index++)));
			}
			output += voice.Process(animationMultiplier, cp1, cp2, traceFunctor);
		}
		
		for (auto& voice : duoVoices) {
			voice.computeDetune(detune);
			if (indexChanged)
			{
				voice.SetTargetFrequency(voiceFrequencies.getFrequency(types::VoiceIndex(index++)));
			}
			output += voice.Process(animationMultiplier, cp1, cp2, traceFunctor);
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
	daisy::Switch travelSelector;
	travelSelector.Init( 
		daisy::seed::D18, 
		0.f, 
		daisy::Switch::Type::TYPE_TOGGLE, 
		daisy::Switch::Polarity::POLARITY_NORMAL,
		daisy::GPIO::Pull::PULLUP);


	const LibDaisyRandom random;
	auto index{0};
	voiceFrequencies.initialize(START_FREQ_RANGE, random);

	//
	//	initialize the voices
	//
	for (auto& voice : trioVoices) 
	{
		voice.Init(
			voiceFrequencies.getFrequency(types::VoiceIndex(index++)),
			types::SampleRate(sample_rate), 
			types::OscillatorFrequency(random(ANIMATION_RATE_RANGE.GetLow().get(), ANIMATION_RATE_RANGE.GetHigh().get())), 
			random
		);
	}

	for (auto& voice : duoVoices) 
	{
		voice.Init(
			voiceFrequencies.getFrequency(types::VoiceIndex(index++)),
			types::SampleRate(sample_rate), 
			types::OscillatorFrequency(random(ANIMATION_RATE_RANGE.GetLow().get(), ANIMATION_RATE_RANGE.GetHigh().get())), 
			random
		);
	}

	filter.Init(sample_rate);
	filter.SetRes(0.0f);

	//	Start ADC and real-time audio processing
	hw.adc.Start();
	hw.StartAudio(AudioCallback);

	valueTravelSelector = travelSelector.RawState();

	//	Loop forever performing non real-time tasks
	while(1) {
		const deepnote::Range OSC_FREQ_RANGE{types::RangeLow(10.f), types::RangeHigh(2000.f)};
		const deepnote::Range FILTER_FREQ_RANGE{types::RangeLow(500.f), types::RangeHigh(15000.f)};
		const deepnote::Range DETUNE_RANGE{types::RangeLow(0.f), types::RangeHigh(5.f)};

		valueDetune = daisysp::fmap(
			hw.adc.GetFloat(DETUNE), 
			DETUNE_RANGE.GetLow().get(), 
			DETUNE_RANGE.GetHigh().get(), 
			daisysp::Mapping::LINEAR);

		valueFilterCutoff = daisysp::fmap(
			hw.adc.GetFloat(FILTER_CUTOFF), 
			FILTER_FREQ_RANGE.GetLow().get(), 
			FILTER_FREQ_RANGE.GetHigh().get(), 
			daisysp::Mapping::LINEAR);

		valueVolume = daisysp::fmap(
			hw.adc.GetFloat(VOLUME), 
			0.f, 
			1.f, 
			daisysp::Mapping::LINEAR);

		hw.PrintLine("Detune " FLT_FMT3, FLT_VAR3(valueDetune));
		hw.PrintLine("Cutoff " FLT_FMT3, FLT_VAR3(valueFilterCutoff));
		hw.PrintLine("Volume " FLT_FMT3, FLT_VAR3(valueVolume));
		hw.PrintLine("Travel %d", valueTravelSelector);

		hw.DelayMs(200);
	}
}