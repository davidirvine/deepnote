cmake_minimum_required(VERSION 3.31)
project(deepnote-test)


include_directories( 
    ../src/unitshapers
    ../src/ranges
    ../src/voice
    ../thirdparty
    ../thirdparty/DaisySP/Source
    ../thirdparty/DaisySP/Source/Utility
)

set(TESTS_SOUCES
    main.cpp
    bezier.cpp
    linear.cpp
    range.cpp
    scaler.cpp
    voice.cpp
)

add_executable(tests 
  ${TESTS_SOUCES}
  ../thirdparty/DaisySP/Source/Synthesis/oscillator.cpp
)

set_target_properties(tests PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  C_STANDARD 11
  C_STANDARD_REQUIRED YES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_test(NAME tests COMMAND tests)
enable_testing()