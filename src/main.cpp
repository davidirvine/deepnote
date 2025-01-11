//
// The THX logo theme consists of 30 voices over seven measures, starting
// in a narrow range, 200 to 400 Hz, and slowly diverging to preselected
// pitches encompassing three octaves. The 30 voices begin at pitches between
// 200 Hz and 400 Hz and arrive at` pre-selected pitches spanning three
// octaves by the fourth measure. The highest pitch is slightly detuned while
// there is double the number of voices of the lowest two pitches.
//

#include "daisy_seed.h"
#include "daisysp.h"
#include "voice/deepnotevoice.hpp"
#include "ranges/range.hpp"

#include "Filters/moogladder.h"

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

const deepnote::Range START_FREQ_RANGE{deepnote::RangeLow(200.f), deepnote::RangeHigh(400.f)};

TrioVoiceType trioVoices[4] = {
	{ START_FREQ_RANGE, 1396.91 },
	{ START_FREQ_RANGE, 1174.66 },
	{ START_FREQ_RANGE, 659.25 },
	{ START_FREQ_RANGE, 587.33 }
};

DuoVoiceType duoVoices[5] = {
 	{ START_FREQ_RANGE, 440.00 },
 	{ START_FREQ_RANGE, 146.83 },
 	{ START_FREQ_RANGE, 110.0 },
 	{ START_FREQ_RANGE, 73.42 },
 	{ START_FREQ_RANGE, 36.71 }
};

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

const deepnote::Range animationRateRange{deepnote::RangeLow(0.05f), deepnote::RangeHigh(1.5f)};
daisysp::MoogLadder filter;

const auto traceFunctor = deepnote::make_named<deepnote::TraceFunctorT>(deepnote::NoopTrace());

void AudioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t bufferSize)
{
	filter.SetFreq(valueFilterCutoff);

	for (size_t bufferIndex = 0; bufferIndex < bufferSize; bufferIndex++) {
		auto output{0.f};
		for (auto& voice : trioVoices) {
			voice.computeDetune(valueDetune);
			output += (voice.Process(traceFunctor) * valueVolume);
		}
		for (auto& voice : duoVoices) {
			voice.computeDetune(valueDetune);
			output += (voice.Process(traceFunctor) * valueVolume);
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


	LibDaisyRandom randomFunctor;

	//
	//	initialize the voices
	//
	for (auto& voice : trioVoices) 
	{
		const auto startFrequency = randomFunctor(animationRateRange.GetLow(), animationRateRange.GetHigh());
		voice.Init(sample_rate, startFrequency, deepnote::make_named<deepnote::RandomFunctorT>(randomFunctor));
	}

	for (auto& voice : duoVoices) 
	{
		const auto startFrequency = randomFunctor(animationRateRange.GetLow(), animationRateRange.GetHigh());
		voice.Init(sample_rate, startFrequency,  deepnote::make_named<deepnote::RandomFunctorT>(randomFunctor));
	}

	filter.Init(sample_rate);
	filter.SetRes(0.0f);

	//	Start ADC and real-time audio processing
	hw.adc.Start();
	hw.StartAudio(AudioCallback);

	valueTravelSelector = travelSelector.RawState();

	//	Loop forever performing non real-time tasks
	while(1) {
		const deepnote::Range OSC_FREQ_RANGE{deepnote::RangeLow(10.f), deepnote::RangeHigh(2000.f)};
		const deepnote::Range FILTER_FREQ_RANGE{deepnote::RangeLow(500.f), deepnote::RangeHigh(15000.f)};
		const deepnote::Range DETUNE_RANGE{deepnote::RangeLow(0.f), deepnote::RangeHigh(5.f)};

		valueDetune = daisysp::fmap(
			hw.adc.GetFloat(DETUNE), 
			DETUNE_RANGE.GetLow(), 
			DETUNE_RANGE.GetHigh(), 
			daisysp::Mapping::LINEAR);

		valueFilterCutoff = daisysp::fmap(
			hw.adc.GetFloat(FILTER_CUTOFF), 
			FILTER_FREQ_RANGE.GetLow(), 
			FILTER_FREQ_RANGE.GetHigh(), 
			daisysp::Mapping::LINEAR);

		valueVolume = daisysp::fmap(
			hw.adc.GetFloat(VOLUME), 
			0.f, 
			1.f, 
			daisysp::Mapping::LINEAR);

		const bool rawState = travelSelector.RawState();
		if (rawState != valueTravelSelector) {
			valueTravelSelector = rawState;
			for (auto& voice : trioVoices) 
			{
				valueTravelSelector ? voice.TransitionToTarget() : voice.TransitionToStart();
				voice.SetAnimationRate(randomFunctor(animationRateRange.GetLow(), animationRateRange.GetHigh()));
			}

			for (auto& voice : duoVoices) 
			{
				valueTravelSelector ? voice.TransitionToTarget() : voice.TransitionToStart();
				voice.SetAnimationRate(randomFunctor(animationRateRange.GetLow(), animationRateRange.GetHigh()));
			}
		}

		hw.PrintLine("Detune " FLT_FMT3, FLT_VAR3(valueDetune));
		hw.PrintLine("Cutoff " FLT_FMT3, FLT_VAR3(valueFilterCutoff));
		hw.PrintLine("Volume " FLT_FMT3, FLT_VAR3(valueVolume));
		hw.PrintLine("Travel %d", valueTravelSelector);

		hw.DelayMs(200);
	}
}