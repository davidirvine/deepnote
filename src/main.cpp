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
#include "deepnotevoice.hpp"
#include "range.hpp"


//	deepnotedrone::LibDaisyRandomFloatGenerator is a random number generator 
//	that uses the libDaisy random number generator.
struct LibDaisyRandomFloatGenerator 
{
	static float GetRandomFloat(float low, float high) 
	{
		return daisy::Random::GetFloat(low, high);
	}	
};

//
//	Voices
//
using DuoVoiceType = DeepnoteVoice<LibDaisyRandomFloatGenerator, 2>;
using TripleVoiceType = DeepnoteVoice<LibDaisyRandomFloatGenerator, 3>;

const int TRIPLE_VOICE_COUNT{6};
const int DUO_VOICE_COUNT{4};
const Range START_FREQ_RANGE{200, 400};

TripleVoiceType tripleVoices[TRIPLE_VOICE_COUNT] = {
	{ START_FREQ_RANGE, 1396.91 },
	{ START_FREQ_RANGE, 1174.66 },
	{ START_FREQ_RANGE, 659.25 },
	{ START_FREQ_RANGE, 587.33 },
	{ START_FREQ_RANGE, 440.00 },
	{ START_FREQ_RANGE, 293.66 }
};

DuoVoiceType duoVoices[DUO_VOICE_COUNT] = {
	{ START_FREQ_RANGE, 146.83 },
	{ START_FREQ_RANGE, 110.0 },
	{ START_FREQ_RANGE, 73.42 },
	{ START_FREQ_RANGE, 36.71 }
};

enum AdcChannelId 
{
	OSC_FREQ = 0,
	FILTER_CUTOFF,
	VOLUME,
	NUM_ADC_CHANNEL
};

//	Values for the knobs and switches
float valueOscFreq{0.f};
float valueFilterCutoff{0.f};
float valueVolume{0.f};
bool valueWaveformSelector{false};


void AudioCallback(daisy::AudioHandle::InputBuffer in, daisy::AudioHandle::OutputBuffer out, size_t bufferSize)
{
	//
	//	Real-time audio processing
	//
	for (size_t bufferIndex = 0; bufferIndex < bufferSize; bufferIndex++) {
		auto output{0.0f};
		for (auto& voice : tripleVoices) {
			output += voice.Process() * valueVolume;
		}
		for (auto& voice : duoVoices) {
			output += voice.Process() * valueVolume;
		}

		out[0][bufferIndex] = output;
		out[1][bufferIndex] = output;
	}
}



int main(void)
{
	daisy::DaisySeed hw;

	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate::SAI_48KHZ);
	//auto sample_rate = hw.AudioSampleRate();

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
	//	oscillator frequenct channel
	adcChannels[OSC_FREQ].InitSingle(daisy::seed::A0);
	//	filter cutoff channel
	adcChannels[FILTER_CUTOFF].InitSingle(daisy::seed::A1);
	//	volume channel
	adcChannels[VOLUME].InitSingle(daisy::seed::A2);
	//	Initialize the ADC
	hw.adc.Init(adcChannels, NUM_ADC_CHANNEL);

	//	waveform selector switch
	daisy::Switch waveformSelector;
	waveformSelector.Init(
		daisy::seed::D18, 
		0.f, 
		daisy::Switch::Type::TYPE_TOGGLE, 
		daisy::Switch::Polarity::POLARITY_NORMAL,
		daisy::GPIO::Pull::PULLUP);


	//
	//	initialize the voices
	//
	// for (auto voice : trippleVoices) {
	// 	voice.Init(sample_rate, 0.1f);
	// }

	// for (auto voice : duoVoices) {
	// 	voice.Init(sample_rate, 0.1f);
	// }


	//	Start the real-time audio processing
	hw.StartAudio(AudioCallback);

	//	Loop forever performing non real-time tasks
	while(1) {
		//
		//	Handle controls
		//
		const Range OSC_FREQ_RANGE{10.f, 2000.f};
		const Range FILTER_FREQ_RANGE{0.f, 500.f};

		valueOscFreq = daisysp::fmap(
			hw.adc.GetFloat(OSC_FREQ), 
			OSC_FREQ_RANGE.getLow(), 
			OSC_FREQ_RANGE.getHigh(), 
			daisysp::Mapping::LINEAR);

		valueFilterCutoff = daisysp::fmap(
			hw.adc.GetFloat(FILTER_CUTOFF), 
			FILTER_FREQ_RANGE.getLow(), 
			FILTER_FREQ_RANGE.getHigh(), 
			daisysp::Mapping::LINEAR);

		valueVolume = daisysp::fmap(
			hw.adc.GetFloat(VOLUME), 
			0.f, 
			1.f, 
			daisysp::Mapping::LINEAR);

		valueWaveformSelector = waveformSelector.Pressed();
	}
}