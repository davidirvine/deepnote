add_executable(unitshapers_test 
    bezier_test.cpp
    linear_test.cpp
)

target_link_libraries(unitshapers_test PRIVATE unitshapers_inf)
AddTests(unitshapers_test)