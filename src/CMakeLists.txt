#
#   some of the included cmake files depend on TARGET being defined
#
set(TARGET deepnote)

set(SOURCES 
    main.cpp
)

add_executable(${TARGET} ${SOURCES})

# Source file specific compiler options
foreach(SRC_FILE ${SOURCES})
    get_filename_component(src_basename ${SRC_FILE} NAME_WLE)
    set_source_files_properties(${SRC_FILE} PROPERTIES 
        COMPILE_FLAGS "-Wa,-a,-ad,-alms=${CMAKE_CURRENT_BINARY_DIR}/${src_basename}.lst"
    )
endforeach()

#
# Link libraries
# Seems odd that we need to specify the libs that libDaisy is dependent on
#
target_link_libraries(${TARGET} PRIVATE
    ${DAISYSP_DIR}/build/libdaisysp.a
    ${LIBDAISY_DIR}/build/libdaisy.a
)

#
# Include directories
#
target_include_directories(${TARGET} PRIVATE
    ranges
    unitshapers
    voice
    ${DAISYSP_DIR}/Source
    ${DAISYSP_DIR}/Source/Utility
    ${LIBDAISY_DIR}/src
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

add_subdirectory(ranges)
add_subdirectory(unitshapers)
add_subdirectory(voice)

include(compiler_linker_options)  
include(post_build_commands)
include(program_commands)
