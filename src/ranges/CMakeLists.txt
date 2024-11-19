add_library(ranges_obj OBJECT
  range.cpp
)

target_include_directories(ranges_obj PUBLIC 
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


