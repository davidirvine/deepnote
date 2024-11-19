# open-ocd -  might need to be adapted for your environment...
set(CHIPSET stm32h7x)
set(OCD openocd)
set(OCD_DIR /usr/local/share/openocd/scripts)
set(PGM_DEVICE interface/stlink.cfg)
set(OCDFLAGS -f ${PGM_DEVICE} -f target/${CHIPSET}.cfg)
set(OCD_PROGRAM ${OCD} -s ${OCD_DIR} ${OCDFLAGS} -c "program ${CMAKE_BINARY_DIR}/bin/${TARGET}.elf verify reset exit")

add_custom_target(program COMMAND ${OCD_PROGRAM})