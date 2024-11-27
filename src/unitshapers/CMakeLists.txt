add_library(unitshapers_inf INTERFACE
  bezier.hpp
  linear.hpp
)

target_include_directories(unitshapers_inf INTERFACE ${CMAKE_CURRENT_LIST_DIR})