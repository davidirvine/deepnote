add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
    ARGS -O ihex
    -S ${TARGET}.elf
    ${TARGET}.hex
    BYPRODUCTS
    ${TARGET}.hex
    COMMENT "Generating HEX image"
    VERBATIM
)

add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
    ARGS -O binary
    -S ${TARGET}.elf
    ${TARGET}.bin
    BYPRODUCTS
    ${TARGET}.bin
    COMMENT "Generating binary image"
    VERBATIM
)