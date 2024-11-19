add_executable(blink 
    main.cpp
)

# Link libraries
target_link_libraries(blink ${CMAKE_BINARY_DIR}/libDaisy/libdaisy.a)
target_link_libraries(blink ${CMAKE_BINARY_DIR}/DaisySP/libdaisysp.a)
target_link_libraries(blink ${CMAKE_BINARY_DIR}/libDaisy/Drivers/libSTM32H7XX_HAL_DRIVER.a)
target_link_libraries(blink ${CMAKE_BINARY_DIR}/libDaisy/Drivers/libCMSIS_DEVICE_H7.a)
target_link_libraries(blink ${CMAKE_BINARY_DIR}/libDaisy/Middlewares/ST/libSTM32_USB_DEVICE_LIBRARY.a)
target_link_libraries(blink ${CMAKE_BINARY_DIR}/libDaisy/Middlewares/ST/libSTM32_USB_HOST_LIBRARY.a)
target_link_libraries(blink ${CMAKE_BINARY_DIR}/libDaisy/Middlewares/Third_Party/FatFs/libFatFs.a)

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