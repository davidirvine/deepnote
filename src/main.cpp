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

using namespace daisy;
using namespace daisy::seed;
using namespace daisysp;
using namespace deepnotedrone;

namespace deepnotedrone {
	const int VOICE_COUNT = 26;

	struct LibDaisyRandomFloatGenerator {
		static float GetRandomFloat(float low, float high) {
			return Random::GetFloat(low, high);
		}	
	};

    struct Util {
        static constexpr float ANALOG_READ_MAX() { return 1023.0; };

        static float invertValue(float a) {
            return Util::ANALOG_READ_MAX() - a;
        }

        static float scaleValue(float a) {
            return a / Util::ANALOG_READ_MAX();
        }
    };

	struct ControlState {
		float oscFreq;
		float filterCutoff;
		float volume;
		bool waveformSelection;
	};

	struct GpioInputs {
		GPIO oscFreqKnob;
		GPIO filterCutoffKnob;
		GPIO volumeKnob;
		GPIO waveformSelector;
	};

	using VoiceType = DeepnoteVoice<LibDaisyRandomFloatGenerator>;

	DaisySeed hw;
	ControlState controlState = {0.0, 0.0, 0.0, false};
	GpioInputs gpioInputs = {};
	VoiceType* voices = nullptr;

	VoiceType* buildVoices(Range startFrequencyRange) {
		VoiceType* voices = new VoiceType[VOICE_COUNT];
		
		voices[0] = { startFrequencyRange, 1396.91, 0.5 };
		voices[1] = { startFrequencyRange, 1396.91, 0.5 };
		voices[2] = { startFrequencyRange, 1396.91, 0.5 };
		voices[3] = { startFrequencyRange, 1174.66, 0.5 };
		voices[4] = { startFrequencyRange, 1174.66, 0.5 };
		voices[5] = { startFrequencyRange, 1174.66, 0.5 };
		voices[6] = { startFrequencyRange, 659.25, 0.5 };
		voices[7] = { startFrequencyRange, 659.25, 0.5 };
		voices[8] = { startFrequencyRange, 659.25, 0.5 };
		voices[9] = { startFrequencyRange, 587.33, 0.5 };
		voices[10] = { startFrequencyRange, 587.33, 0.5 };
		voices[11] = { startFrequencyRange, 587.33, 0.5 };
		voices[12] = { startFrequencyRange, 440.00, 0.5 };
		voices[13] = { startFrequencyRange, 440.00, 0.5 };
		voices[14] = { startFrequencyRange, 440.00, 0.5 };
		voices[15] = { startFrequencyRange, 293.66, 0.5 };
		voices[16] = { startFrequencyRange, 293.66, 0.5 };
		voices[17] = { startFrequencyRange, 293.66, 0.5 };
		voices[18] = { startFrequencyRange, 146.83, 0.0 };
		voices[19] = { startFrequencyRange, 146.83, 0.0 };
		voices[20] = { startFrequencyRange, 110.0, 0.0 };
		voices[21] = { startFrequencyRange, 110.0, 0.0 };
		voices[22] = { startFrequencyRange, 73.42, 0.0 };
		voices[23] = { startFrequencyRange, 73.42, 0.0 };
		voices[24] = { startFrequencyRange, 36.71, 0.0 };
		voices[25] = { startFrequencyRange, 36.71, 0.0 };

		return voices;
	}


	GpioInputs setupGpioPins() {
		GpioInputs inputs;
		inputs.oscFreqKnob.Init(A0, GPIO::Mode::INPUT);
		inputs.filterCutoffKnob.Init(A1, GPIO::Mode::INPUT);
		inputs.volumeKnob.Init(A2, GPIO::Mode::INPUT);
		inputs.waveformSelector.Init(D18, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
		return inputs;
	}

	float readOscKnob() {
		return fmap(Util::scaleValue(gpioInputs.oscFreqKnob.Read()), 10, 2000, Mapping::LINEAR);
	}

	float readFilterKnob() {
		return fmap(Util::scaleValue(gpioInputs.filterCutoffKnob.Read()), 0, 500, Mapping::LINEAR);
	}

	float readVolumeKnob() {
		return 1 - Util::scaleValue(gpioInputs.volumeKnob.Read());
	}

	bool readWaveformSelector() {
		return gpioInputs.waveformSelector.Read();
	}

	struct ControlState getControlState() {
		struct ControlState controlState = {
			readOscKnob(),
			readFilterKnob(),
			readVolumeKnob(),
			readWaveformSelector()
		};
		return controlState;
	}
} // namespace deepnotedrone


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t bufferSize)
{
	for (size_t bufferIndex = 0; bufferIndex < bufferSize; bufferIndex++) {
		auto output = 0.0;
		for (size_t voiceIndex = 0; voiceIndex < VOICE_COUNT; ++voiceIndex) {
			output += voices[voiceIndex].Process();
		}
		out[0][bufferIndex] = output * controlState.volume;
		out[1][bufferIndex] = output * controlState.volume;
	}
}


int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate::SAI_48KHZ);
	auto sample_rate = hw.AudioSampleRate();

	//
	//	coinfigure the GPIO pins for the knobs and switches
	//
	gpioInputs = setupGpioPins();

	//
	//	coinstruct and intiaize the voices
	//
	voices = buildVoices(Range(200, 400));
	for (auto i = 0; i < VOICE_COUNT; ++i) {
		voices[i].Init(
			sample_rate, 
			Random::GetFloat(10.0, 60.0),  	//	detune rate Hz
			Random::GetFloat(0.0, 1.0)		//	animation rate Hz
		);	
	}
	
	hw.StartAudio(AudioCallback);

	while(1) {
		controlState = getControlState();
	}
}

