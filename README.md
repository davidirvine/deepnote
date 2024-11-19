# Crosscompile

This project serves as a template for Daisy Seed development using VSCode. 

## Submodules

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
make program
```

## Source Level Debugging
A launch configuration is provided which enables source level debugging using a JTAG debug probe.
