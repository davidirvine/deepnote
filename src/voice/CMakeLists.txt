add_library(voice_obj OBJECT
  deepnotevoice.cpp
)

target_include_directories(voice_obj PUBLIC 
  ../ranges
  ../unitshapers
  ${DAISYSP_DIR}/Source
)
