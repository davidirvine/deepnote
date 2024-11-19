add_library(unitshapers_lib INTERFACE
  bezier.hpp
  linear.hpp
)

target_include_directories(unitshapers_lib INTERFACE 
  ${CMAKE_CURRENT_LIST_DIR}
)
