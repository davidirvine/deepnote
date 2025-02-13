# deepnote

A Daisy Seed synth thing inspired by THX Deep Note. https://www.thx.com/deepnote/

This repo uses Git submodules so when cloning you'll probably want to use the `--recurse-submodules` option. If you forgot or want to update the submodules recursively use `git submodule update --init --recursive`.


## Build Setup
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

## Building
```
make
```

## Device Programming
```
make program-dfu
```

## Source Level Debugging
A launch configuration is provided which enables source level debugging using a JTAG debug probe.

## Unit Tests
Unit tests follow the pattern found in `libDaisy`. Unit tests are complied and run natively not cross compiled for the Daisy Seed. This limits what you can unit test since the code under test can not be dependent on `libDaisy`.

To build the unit tests:
```
cd test
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

Unit tests are compiled with the `-g` option. A sample source level unit test debug configuration can be found in `.vscode/launch.json`.
