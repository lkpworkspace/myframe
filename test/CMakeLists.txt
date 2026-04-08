cmake_minimum_required(VERSION 3.10)

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
if (MYFRAME_ENABLE_ASAN)
  add_compile_options(
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-fsanitize=address>"
  )
  add_link_options(
    "$<$<CXX_COMPILER_ID:GNU,Clang>:-fsanitize=address>"
  )
endif ()

### test bin
add_executable(test_hello test_hello.cpp)
target_link_libraries(test_hello
    myframe
)

add_executable(test_common test_common.cpp)
target_link_libraries(test_common
    myframe
)

add_executable(test_actor_run_queue test_actor_run_queue.cpp)
target_link_libraries(test_actor_run_queue
    myframe
)

add_executable(test_app_send test_app_send.cpp)
target_link_libraries(test_app_send
    myframe
)

add_executable(test_app_send_req test_app_send_req.cpp)
target_link_libraries(test_app_send_req
    myframe
)

add_executable(test_performance_trans1_cost test_performance_trans1_cost.cpp)
target_link_libraries(test_performance_trans1_cost
    myframe
)

add_executable(test_performance_trans10_cost test_performance_trans10_cost.cpp)
target_link_libraries(test_performance_trans10_cost
    myframe
)

add_executable(test_performance_trans1_fullspeed test_performance_trans1_fullspeed.cpp)
target_link_libraries(test_performance_trans1_fullspeed
    myframe
)

add_executable(test_performance_trans20_fullspeed test_performance_trans20_fullspeed.cpp)
target_link_libraries(test_performance_trans20_fullspeed
    myframe
)

add_executable(test_performance_trans100_fullspeed test_performance_trans100_fullspeed.cpp)
target_link_libraries(test_performance_trans100_fullspeed
    myframe
)

### install
INSTALL(TARGETS
    test_hello
    test_common
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
