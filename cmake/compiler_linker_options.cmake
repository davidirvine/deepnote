set(LINKER_SCRIPT ${LIBDAISY_DIR}/core/STM32H750IB_flash.lds)


set_target_properties(${TARGET} PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  C_STANDARD 11
  C_STANDARD_REQUIRED YES
  SUFFIX ".elf"
  LINK_DEPENDS ${LINKER_SCRIPT}
)

add_compile_definitions($<$<CONFIG:Release,MinSizeRel>:NDEBUG>)

target_link_options(${TARGET} PUBLIC
  LINKER:-T,${LINKER_SCRIPT}
  $<$<CONFIG:Debug>:LINKER:-Map=${TARGET}.map>
  $<$<CONFIG:Debug>:LINKER:--cref>
  $<$<CONFIG:Release,MinSizeRel,RelWithDebInfo>:LINKER:-flto>
  LINKER:--gc-sections
  LINKER:--check-sections
  LINKER:--unresolved-symbols=report-all
  LINKER:--warn-common
  $<$<CXX_COMPILER_ID:GNU>:LINKER:--warn-section-align>
  # Currently a GSoC project to port this to LLD
  $<$<CXX_COMPILER_ID:GNU>:LINKER:--print-memory-usage>
)

target_compile_options(${TARGET} PUBLIC
  $<$<CONFIG:Debug>:-Og>
  $<$<CONFIG:Release>:-O3>
  $<$<CONFIG:MinSizeRel>:-Os>
  $<$<CONFIG:Release,MinSizeRel,RelWithDebInfo>:-flto>
  $<$<CONFIG:Debug,RelWithDebInfo>:-ggdb3>
  -Wall
  -Wno-attributes
  -Wno-strict-aliasing
  -Wno-maybe-uninitialized
  -Wno-missing-attributes
  -Wno-stringop-overflow
  -Wno-error=reorder
  -Wno-error=sign-compare
  -fexceptions
  -DQ_DONT_USE_THREADS=1
  $<$<COMPILE_LANGUAGE:CXX>:-Wno-register>
  # At same point this generator should be removed, but the clang build isn't there yet
  $<$<CXX_COMPILER_ID:GNU>:-Werror>
  # These are explicitly for startup_stm32h750xx.c
  $<$<CXX_COMPILER_ID:GNU>:-Wno-attributes>
  $<$<CXX_COMPILER_ID:GNU>:-Wno-missing-attributes>
)