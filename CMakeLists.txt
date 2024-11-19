cmake_minimum_required(VERSION 3.10)

cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0135 NEW)
cmake_policy(SET CMP0152 NEW)

include (${CMAKE_CURRENT_LIST_DIR}/cmake/toolchains/autodetect.cmake)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project(Blink VERSION 0.1.0 LANGUAGES CXX)
#enable_testing()

#
# Daisy specific options
#
set(DAISY_STORAGE flash CACHE STRING "Select the storage destination of the executable.")

add_subdirectory(src bin)
#add_subdirectory(test)

