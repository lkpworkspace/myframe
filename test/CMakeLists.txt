### configure file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/test_config.h @ONLY)

### link static libs
if (glog_lib_type STREQUAL "STATIC_LIBRARY")
  link_libraries(
    glog::glog
  )
endif ()
if (jsoncpp_lib_type STREQUAL "STATIC_LIBRARY")
  link_libraries(
    JsonCpp::JsonCpp
  )
endif ()

### test bin
add_executable(test_hello test_hello.cpp)
target_link_libraries(test_hello
    myframe
)
add_test(NAME test_hello COMMAND ${CMAKE_INSTALL_PREFIX}/bin/test_hello)

add_executable(test_common test_common.cpp)
target_link_libraries(test_common
    myframe
)
add_test(NAME test_common COMMAND ${CMAKE_INSTALL_PREFIX}/bin/test_common)

add_executable(test_arguments test_arguments.cpp)
target_link_libraries(test_arguments
    myframe
)
add_test(NAME test_arguments COMMAND ${CMAKE_INSTALL_PREFIX}/bin/test_arguments)

add_executable(test_actor_run_queue test_actor_run_queue.cpp)
target_link_libraries(test_actor_run_queue
    myframe
)
add_test(NAME test_actor_run_queue COMMAND ${CMAKE_INSTALL_PREFIX}/bin/test_actor_run_queue)

add_executable(test_app_send test_app_send.cpp)
target_link_libraries(test_app_send
    myframe
)
add_test(NAME test_app_send COMMAND ${CMAKE_INSTALL_PREFIX}/bin/test_app_send)

add_executable(test_app_send_req test_app_send_req.cpp)
target_link_libraries(test_app_send_req
    myframe
)
add_test(NAME test_app_send_req COMMAND ${CMAKE_INSTALL_PREFIX}/bin/test_app_send_req)

add_executable(test_performance_trans1_cost test_performance_trans1_cost.cpp)
target_link_libraries(test_performance_trans1_cost
    myframe
)
add_test(NAME test_performance_trans1_cost COMMAND ${CMAKE_INSTALL_PREFIX}/bin/test_performance_trans1_cost)

add_executable(test_performance_trans10_cost test_performance_trans10_cost.cpp)
target_link_libraries(test_performance_trans10_cost
    myframe
)
add_test(NAME test_performance_trans10_cost COMMAND ${CMAKE_INSTALL_PREFIX}/bin/test_performance_trans10_cost)

add_executable(test_performance_trans1_fullspeed test_performance_trans1_fullspeed.cpp)
target_link_libraries(test_performance_trans1_fullspeed
    myframe
)
add_test(NAME test_performance_trans1_fullspeed COMMAND ${CMAKE_INSTALL_PREFIX}/bin/test_performance_trans1_fullspeed)

add_executable(test_performance_trans20_fullspeed test_performance_trans20_fullspeed.cpp)
target_link_libraries(test_performance_trans20_fullspeed
    myframe
)
add_test(NAME test_performance_trans20_fullspeed COMMAND ${CMAKE_INSTALL_PREFIX}/bin/test_performance_trans20_fullspeed)

add_executable(test_performance_trans100_fullspeed test_performance_trans100_fullspeed.cpp)
target_link_libraries(test_performance_trans100_fullspeed
    myframe
)
add_test(NAME test_performance_trans100_fullspeed COMMAND ${CMAKE_INSTALL_PREFIX}/bin/test_performance_trans100_fullspeed)

### install
INSTALL(TARGETS
    test_hello
    test_common
    test_arguments
    test_actor_run_queue
    test_app_send
    test_app_send_req
    test_performance_trans1_cost
    test_performance_trans10_cost
    test_performance_trans1_fullspeed
    test_performance_trans20_fullspeed
    test_performance_trans100_fullspeed
    LIBRARY DESTINATION ${MYFRAME_LIB_DIR}
    ARCHIVE DESTINATION ${MYFRAME_LIB_DIR}
    RUNTIME DESTINATION ${MYFRAME_BIN_DIR}
)
install(DIRECTORY test_data DESTINATION .)
