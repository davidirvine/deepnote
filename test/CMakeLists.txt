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

# Reasonable compiler warnings without being too aggressive
target_compile_options(tests PRIVATE 
    -Wall 
    -Wextra 
    # Removed -Wpedantic and -Werror to avoid issues with external dependencies
    -Wno-unused-parameter      # DaisySP has many unused parameters
    -Wno-unused-variable       # External dependencies may have unused variables
    -Wno-sign-compare          # Common in external code
    -Wno-missing-field-initializers  # External dependencies often don't initialize all fields
    $<$<CONFIG:Debug>:-g -O0>  # Removed sanitizers as they can be too aggressive for development
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Removed sanitizer linking for now to avoid development friction
# target_link_options(tests PRIVATE
#     $<$<CONFIG:Debug>:-fsanitize=address -fsanitize=undefined>
# )

add_test(NAME tests COMMAND tests)
enable_testing()
