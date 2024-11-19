# Crosscompile

This project serves as a template for Daisy Seed development using VSCode. 

`libDaisy` and `DaisySP` libraries are dependencies. You'll need to adjust their filesystem locations in `src/CMakeLists.txt` and `.vscode/c_cpp_properties.json`. This will go away once these repos are added as submodules.

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
