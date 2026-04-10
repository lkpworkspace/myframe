CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

add_compile_definitions(${PROJECT_NAME}_EXPORTS)

### link static lib
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

### actor
add_library(example_actor_helloworld SHARED example_actor_helloworld.cpp)
target_link_libraries(example_actor_helloworld ${PROJECT_NAME})
add_library(example_actor_timer SHARED example_actor_timer.cpp)
target_link_libraries(example_actor_timer ${PROJECT_NAME})
add_library(example_actor_serial SHARED example_actor_serial.cpp)
target_link_libraries(example_actor_serial ${PROJECT_NAME})
add_library(example_actor_concurrent SHARED example_actor_concurrent.cpp)
target_link_libraries(example_actor_concurrent ${PROJECT_NAME})
add_library(example_actor_pub_sub SHARED example_actor_pub_sub.cpp)
target_link_libraries(example_actor_pub_sub ${PROJECT_NAME})

### worker
add_library(example_worker_quit SHARED example_worker_quit.cpp)
target_link_libraries(example_worker_quit ${PROJECT_NAME})
add_library(example_worker_io SHARED example_worker_io.cpp)
target_link_libraries(example_worker_io ${PROJECT_NAME})

### base
add_library(example_config SHARED example_config.cpp)
target_link_libraries(example_config ${PROJECT_NAME})
add_library(example_thread_quit SHARED example_thread_quit.cpp)
target_link_libraries(example_thread_quit ${PROJECT_NAME})
add_library(example_trans_msg SHARED example_trans_msg.cpp)
target_link_libraries(example_trans_msg ${PROJECT_NAME})
add_library(example_trans_obj SHARED example_trans_obj.cpp)
target_link_libraries(example_trans_obj ${PROJECT_NAME})

### install
file(GLOB conf_files "*.json")
install(FILES
  ${conf_files}
  PERMISSIONS
  OWNER_READ OWNER_WRITE
  GROUP_READ
  WORLD_READ
  DESTINATION ${MYFRAME_SERVICE_DIR}
)
install(TARGETS
  example_actor_helloworld
  example_actor_timer
  example_actor_serial
  example_actor_concurrent
  example_actor_pub_sub
  example_worker_quit
  example_worker_io
  example_config
  example_thread_quit
  example_trans_msg
  example_trans_obj
  LIBRARY DESTINATION ${MYFRAME_LIB_DIR}
  ARCHIVE DESTINATION ${MYFRAME_LIB_DIR}
  RUNTIME DESTINATION ${MYFRAME_BIN_DIR}
)
