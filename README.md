# deepnote

An implementaiton of a synth void inspired by [THX Deep Note](https://www.thx.com/deepnote/).

The voice is written as a header only C++14 library. [DaisySP](https://github.com/electro-smith/DaisySP) is used for the oscillator implementaions.

[deepnote-rack](https://github.com/davidirvine/deepnote-rack) wraps a VCVRack module around a `deepnote` voice providing CV control over voice parameters. 

[deepnote-seed](https://github.com/davidirvine/deepnote-seed) embeds a `deepnote` voice into Daisy Seed hardware providing physical control over voice parametes. 


This repo uses Git submodules so after cloning this repo:
```
git submodule init
git submodule update --remote --recursive
```

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
