cmake_minimum_required(VERSION 3.16)
project(deepnote-test)

find_package(Threads REQUIRED)
add_subdirectory(googletest)
include(GoogleTest)

set(MODULE_DIR ../src)
set(DAISYSP_DIR ${CMAKE_CURRENT_LIST_DIR}/../DaisySP)

# if we're not cross-compiling, we can do unit tests
add_library(deepnote STATIC
  ${CMAKE_CURRENT_LIST_DIR}/${MODULE_DIR}/ranges/range.cpp
  ${DAISYSP_DIR}/Source/Synthesis/oscillator.cpp
)
target_include_directories(deepnote PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/${MODULE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/${MODULE_DIR}/unitshapers
    ${CMAKE_CURRENT_LIST_DIR}/${MODULE_DIR}/ranges
    ${CMAKE_CURRENT_LIST_DIR}/${MODULE_DIR}/voice
    ${DAISYSP_DIR}/Source
    ${DAISYSP_DIR}/Source/Utility
)
target_compile_definitions(deepnote PUBLIC UNIT_TEST)

target_link_libraries(deepnote PUBLIC 
    GTest::gtest_main
)

enable_testing()

# Add a gtest
function(add_gtest source_file)
  cmake_path(GET source_file STEM test_name)

  # Create the target
  add_executable(${test_name} ${source_file})

  # Link it against the required libraries
  target_link_libraries(${test_name} PUBLIC
    deepnote
    Threads::Threads
    GTest::gtest_main
  )

  # Set any compile definitions
  target_compile_definitions(${test_name} PRIVATE
    UNIT_TEST
  )

  target_compile_options(${test_name} PUBLIC
    -g
  )

  # Set the properties
  set_target_properties(${test_name} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )

  # Add the target as a test
  #add_test(NAME ${test_name} COMMAND ${test_name} --verbose)

  # Discover unit tests inside it
  gtest_discover_tests(${test_name})
endfunction(add_gtest)

# Generate test executables from files ending in *_gtest.cpp
function(autogen_gtests test_folder)
  file(GLOB_RECURSE specs ${test_folder}/*_gtest.cpp)

  foreach(spec IN LISTS specs)
    add_gtest(${spec})
  endforeach()
endfunction()

autogen_gtests(${CMAKE_CURRENT_LIST_DIR} deepnote)