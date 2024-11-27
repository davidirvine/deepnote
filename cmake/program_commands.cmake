set(DFU dfu-util)
set(OCD openocd)

# program-bootloader
# program the bootloader using dfu-util
# this is required if you want to use the Daisy bootloader and have your
# program installed and run from SRAM or QSPI. See compiler_linker_options.cmake
set(BOOTLOADER_BIN ${LIBDAISY_DIR}/core/dsy_bootloader_v6_2-intdfu-2000ms.bin)
set(DFU_PROGRAM_BOOTLOADER ${DFU} -a 0 -s ${INTERNAL_ADDRESS}:leave -D ${BOOTLOADER_BIN} -d ,0483:${STM_PID})
add_custom_target(program-bootloader COMMAND ${DFU_PROGRAM_BOOTLOADER})

# program-dfu
# program the target using dfu-util
set(DFU_PROGRAM ${DFU} -a 0 -s ${FLASH_ADDRESS}:leave -D ${TARGET_BIN} -d ,0483:${USBPID})
add_custom_target(program-dfu COMMAND ${DFU_PROGRAM})


if ("x_${EXECUTABLE_STORAGE_LOCATION}" STREQUAL "x_BOOT_FLASH")
    # program-ocd
    # program the target via JTAG probe using openocd
    set(CHIPSET stm32h7x)
    set(OCD_DIR /usr/local/share/openocd/scripts)
    set(PGM_DEVICE interface/stlink.cfg)
    set(OCDFLAGS -f ${PGM_DEVICE} -f target/${CHIPSET}.cfg)
    set(OCD_PROGRAM ${OCD} -s ${OCD_DIR} ${OCDFLAGS} -c "program ${TARGET_ELF} verify reset exit")
    add_custom_target(program-ocd COMMAND ${OCD_PROGRAM})
endif()