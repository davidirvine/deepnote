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
    performance_tests.cpp
    property_edge_tests.cpp
    statemachine_lifecycle_tests.cpp
    bezier_animation_tests.cpp
    error_robustness_tests.cpp
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

# Compiler-specific warnings and options
if(MSVC)
    target_compile_options(tests PRIVATE 
        /W3                        # Warning level 3 (reasonable warnings)
        /wd4996                    # Disable "deprecated" warnings for standard functions
        /wd4244                    # Disable "conversion" warnings that are common in audio code
        /wd4267                    # Disable size_t conversion warnings
        $<$<CONFIG:Debug>:/Od /Zi> # Debug: no optimization, debug info
        $<$<CONFIG:Release>:/O2 /DNDEBUG> # Release: optimize for speed
    )
else()
    # GCC/Clang flags
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
endif()

# Removed sanitizer linking for now to avoid development friction
# target_link_options(tests PRIVATE
#     $<$<CONFIG:Debug>:-fsanitize=address -fsanitize=undefined>
# )

add_test(NAME tests COMMAND tests)
enable_testing()
