add_executable(blink 
    main.cpp
)

# Library Locations
#   TODO: there should be a better way to do this so relative paths aren't in this file
#   TODO: perhaps cmake profiles?
file(REAL_PATH ../../electrosmith/DaisySP DAISYSP_DIR BASE_DIRECTORY "${CMAKE_SOURCE_DIR}")
file(REAL_PATH ../../electrosmith/libDaisy LIBDAISY_DIR BASE_DIRECTORY "${CMAKE_SOURCE_DIR}")

# Link libraries
target_link_libraries(blink ${LIBDAISY_DIR}/build/libdaisy.a)
target_link_libraries(blink ${DAISYSP_DIR}/build/libdaisysp.a)

target_include_directories(blink PRIVATE
    ${DAISYSP_DIR}/Source
    ${LIBDAISY_DIR}/src/
    ${LIBDAISY_DIR}/src/sys
    ${LIBDAISY_DIR}/src/usbh
    ${LIBDAISY_DIR}/src/Drivers/STM32H7xx_HAL_Driver/Inc
    ${LIBDAISY_DIR}/Middlewares/ST/STM32_USB_Host_Library/Core/Inc
    ${LIBDAISY_DIR}/Middlewares/Third_Party/FatFs/src
    ${LIBDAISY_DIR}/Drivers/CMSIS_5/CMSIS/Core/Include
    ${LIBDAISY_DIR}/Drivers/CMSIS-Device/ST/STM32H7xx/Include
    ${LIBDAISY_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
    ${LIBDAISY_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy
)

# linker_compiler_options depends on TARGET being defined
set(TARGET blink)
include(compiler_linker_options)  
include(post_build_commands)
include(add_program_target)