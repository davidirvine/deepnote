add_library(voice_inf INTERFACE
  deepnotevoice.hpp
  frequencytable.hpp
)

target_include_directories(voice_inf INTERFACE 
  ${CMAKE_CURRENT_LIST_DIR}
  ../ranges
  ../unitshapers
  ${DAISYSP_DIR}/Source
)
