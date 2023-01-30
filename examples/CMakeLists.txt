CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

### actor
add_library(example_actor_helloworld SHARED example_actor_helloworld.cpp)
target_link_libraries(example_actor_helloworld myframe)
add_library(example_actor_timer SHARED example_actor_timer.cpp)
target_link_libraries(example_actor_timer myframe)
add_library(example_actor_serial SHARED example_actor_serial.cpp)
target_link_libraries(example_actor_serial myframe)
add_library(example_actor_concurrent SHARED example_actor_concurrent.cpp)
target_link_libraries(example_actor_concurrent myframe)

### worker
add_library(example_worker_publish SHARED example_worker_publish.cpp)
target_link_libraries(example_worker_publish myframe)
add_library(example_worker_talk SHARED example_worker_talk.cpp)
target_link_libraries(example_worker_talk myframe)
add_library(example_worker_actor_interactive SHARED example_worker_actor_interactive.cpp)
target_link_libraries(example_worker_actor_interactive myframe)
add_library(example_worker_interactive_with_3rd_frame SHARED example_worker_interactive_with_3rd_frame.cpp)
target_link_libraries(example_worker_interactive_with_3rd_frame myframe)
add_library(example_worker_quit SHARED example_worker_quit.cpp)
target_link_libraries(example_worker_quit myframe)

add_library(example_config SHARED example_config.cpp)
target_link_libraries(example_config myframe)

### install
FILE(GLOB conf_files "*.json")
INSTALL(FILES 
    ${conf_files}
    PERMISSIONS
    OWNER_READ
    GROUP_READ
    WORLD_READ
    DESTINATION ${MYFRAME_SERVICE_DIR}
)
INSTALL(TARGETS 
    example_actor_helloworld
    example_actor_timer
    example_actor_serial
    example_actor_concurrent
    example_worker_actor_interactive 
    example_worker_publish
    example_worker_talk 
    example_worker_interactive_with_3rd_frame
    example_worker_quit
    example_config
    LIBRARY DESTINATION ${MYFRAME_LIB_DIR}
    ARCHIVE DESTINATION ${MYFRAME_LIB_DIR}
    RUNTIME DESTINATION ${MYFRAME_BIN_DIR}
)
