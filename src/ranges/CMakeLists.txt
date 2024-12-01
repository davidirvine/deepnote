add_library(ranges_inf INTERFACE
  range.hpp
  scaler.hpp
)

target_include_directories(ranges_inf INTERFACE 
  ${CMAKE_CURRENT_LIST_DIR}
)