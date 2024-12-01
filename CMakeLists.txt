cmake_minimum_required(VERSION 3.31)

cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0135 NEW)
cmake_policy(SET CMP0152 NEW)

include (${CMAKE_CURRENT_LIST_DIR}/cmake/toolchains/autodetect.cmake)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Library Locations
set(LIBDAISY_DIR ${CMAKE_SOURCE_DIR}/thirdparty/libDaisy)
set(DAISYSP_DIR ${CMAKE_SOURCE_DIR}/thirdparty/DaisySP)

project(Deepnote VERSION 0.1.0 LANGUAGES CXX)

#
# Where shuld this progeam be installed and run from? Possible values are:
# BOOT_FLASH, BOOT_SRAM, BOOT_QSPI
#
# Both BOOT_SRAM and BOOT_QSPI require the Daisy bootloader to be installed.
#
# Use "make program-bootloader" to install the bootloader
# before installing your program
#
# See: add_program_commands.cmake
#
set(EXECUTABLE_STORAGE_LOCATION BOOT_FLASH)

include(add_external_projects)
add_subdirectory(src bin)