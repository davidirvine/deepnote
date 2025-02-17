# deepnote

An implementaiton of a synth void inspired by [THX Deep Note](https://www.thx.com/deepnote/).

The voice is written as a header only C++14 library. [DaisySP](https://github.com/electro-smith/DaisySP) is used for the oscillator implementaions.

[deepnote-rack](https://github.com/davidirvine/deepnote-rack) wraps a VCVRack module around a `deepnote` voice providing CV control over voice parameters. 

[deepnote-seed](https://github.com/davidirvine/deepnote-seed) embeds a `deepnote` voice into Daisy Seed hardware providing physical control over voice parametes. 

## Overview

Each `deepnote::DeepnoteVoice` has: 
- 1 or more oscillators
- a start, target, and current frequency
- an animation LFO
- an animation scaler

The animation LFO defines how quickly current frequency transitions from start to target frequency. LFO output is mapped via an animation scaler to a point in the start to target frequency range. This enables mapping beyond simple linear mapping.

The oscilators of a `deepnote::DeepnoteVoice` can be detuned (defaults to no/0Hz detune) and are of type `daisysp::Oscillator::WAVE_POLYBLEP_SAW`.

The `deepnote::DeepnoteVoice::init` method must be called before using an instance of `deepnote::DeepnoteVoice`. This method requires the caller to specify start frequency, sample rate, and animation LFO frequency.

A `deepnote::BezierUnitShaper` is used for the animation scaler. The shape of the bezier curve can be manipulated via 2 control points. Values for these control points along with a multiplyer for the animation LFO frequency are required arguments to the `deepnote::DeepnoteVoice::process` method.

`deepnote::DeepnoteVoice::process` should be called from your audio loop to generate a single audio sample. 

## Strong Types

There are a lot of variables, function parameters, etc of type float. Strong types are used to provide an easy to understand interface and provide structure to the sea of floats. These strong types are defined in the `deepnote::nt` namespace and utilize `deepnote::NamedType` found in `src/util/namedtype.hpp`.

## Building and Running Unit Tests

Unit tests are implemented using [doctest](https://github.com/doctest/doctest). To build the unit tests:

```
cd test
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..

make
```

To run the unit tests:
```
cd test
./build/bin/tests
```

The `Unit Test Debug` configuration can be used for source debugging of the unit tests.
