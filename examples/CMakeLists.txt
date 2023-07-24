CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

### actor
add_library(example_actor_helloworld SHARED example_actor_helloworld.cpp)
target_link_libraries(example_actor_helloworld ${PROJECT_NAME})
add_library(example_actor_timer SHARED example_actor_timer.cpp)
target_link_libraries(example_actor_timer ${PROJECT_NAME})
add_library(example_actor_serial SHARED example_actor_serial.cpp)
target_link_libraries(example_actor_serial ${PROJECT_NAME})
add_library(example_actor_concurrent SHARED example_actor_concurrent.cpp)
target_link_libraries(example_actor_concurrent ${PROJECT_NAME})

### worker
add_library(example_worker_publish SHARED example_worker_publish.cpp)
target_link_libraries(example_worker_publish ${PROJECT_NAME})
add_library(example_worker_talk SHARED example_worker_talk.cpp)
target_link_libraries(example_worker_talk ${PROJECT_NAME})
add_library(example_worker_actor_interactive SHARED example_worker_actor_interactive.cpp)
target_link_libraries(example_worker_actor_interactive ${PROJECT_NAME})
add_library(example_worker_interactive_with_3rd_frame SHARED example_worker_interactive_with_3rd_frame.cpp)
target_link_libraries(example_worker_interactive_with_3rd_frame ${PROJECT_NAME})
add_library(example_worker_quit SHARED example_worker_quit.cpp)
target_link_libraries(example_worker_quit ${PROJECT_NAME})

add_library(example_config SHARED example_config.cpp)
target_link_libraries(example_config ${PROJECT_NAME})
add_library(example_trans_obj SHARED example_trans_obj.cpp)
target_link_libraries(example_trans_obj ${PROJECT_NAME})

### install
FILE(GLOB conf_files "*.json")
INSTALL(FILES 
    ${conf_files}
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
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
    example_trans_obj
    LIBRARY DESTINATION ${MYFRAME_LIB_DIR}
    ARCHIVE DESTINATION ${MYFRAME_LIB_DIR}
    RUNTIME DESTINATION ${MYFRAME_BIN_DIR}
)
