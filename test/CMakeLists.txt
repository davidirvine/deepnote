cmake_minimum_required(VERSION 3.31)
project(deepnote-test)

include_directories( 
    ../src
    ../thirdparty
    ../thirdparty/DaisySP/Source
    ../thirdparty/DaisySP/Source/Utility
)

set(TESTS_SOUCES
    bezier.cpp
    freqtable.cpp
    linear.cpp
    main.cpp
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

# Enhanced compiler warnings and debugging options
target_compile_options(tests PRIVATE 
    -Wall 
    -Wextra 
    -Wpedantic 
    -Werror
    -Wno-unused-parameter  # DaisySP has many unused parameters
    $<$<CONFIG:Debug>:-g -O0 -fsanitize=address -fsanitize=undefined>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Link sanitizers in debug mode
target_link_options(tests PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address -fsanitize=undefined>
)

add_test(NAME tests COMMAND tests)
enable_testing()
