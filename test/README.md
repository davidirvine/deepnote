# Unit Tests

These tests are compiled using the host toolchain. This means that you are limited to code that's compatible with the host runtime.

Building tests:

```
cd test
mkdir build
cd build
cmake ..
```

Running tests:
```
ctest
```
or to get finer grained result output:
```
./bin/tests
```

Tests are implemented using `doctest`. Integration with `cmake` could be improved throught the custom `cmake` commands provided with `doctest`.