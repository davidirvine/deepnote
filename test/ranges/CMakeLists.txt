add_executable(ranges_test 
    range_test.cpp
    scaler_test.cpp
)

target_link_libraries(ranges_test PRIVATE ranges_static)
AddTests(ranges_test)
EnableCoverage(ranges_obj)