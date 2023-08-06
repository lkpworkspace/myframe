cmake_minimum_required(VERSION 3.10)

### configure file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/performance_test_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/performance_test_config.h @ONLY)

### test bin
add_executable(app_send_test app_send_test.cpp)
target_link_libraries(app_send_test
    myframe
)

add_executable(performance_trans1_cost_test performance_trans1_cost_test.cpp)
target_link_libraries(performance_trans1_cost_test
    myframe
)

add_executable(performance_trans10_cost_test performance_trans10_cost_test.cpp)
target_link_libraries(performance_trans10_cost_test
    myframe
)

add_executable(performance_trans1_fullspeed_test performance_trans1_fullspeed_test.cpp)
target_link_libraries(performance_trans1_fullspeed_test
    myframe
)

add_executable(performance_trans20_fullspeed_test performance_trans20_fullspeed_test.cpp)
target_link_libraries(performance_trans20_fullspeed_test
    myframe
)

add_executable(performance_trans100_fullspeed_test performance_trans100_fullspeed_test.cpp)
target_link_libraries(performance_trans100_fullspeed_test
    myframe
)

### install
INSTALL(TARGETS
    app_send_test
    performance_trans1_cost_test
    performance_trans10_cost_test
    performance_trans1_fullspeed_test
    performance_trans20_fullspeed_test
    performance_trans100_fullspeed_test
    LIBRARY DESTINATION ${MYFRAME_LIB_DIR}
    ARCHIVE DESTINATION ${MYFRAME_LIB_DIR}
    RUNTIME DESTINATION ${MYFRAME_TEST_DIR}
)
