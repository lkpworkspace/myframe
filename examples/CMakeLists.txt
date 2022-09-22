CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

### Actor
add_library(example_actor_hello_world SHARED ExampleActorHelloWorld.cpp)
add_library(example_actor_timer SHARED ExampleActorTimer.cpp)
add_library(example_actor_serial SHARED ExampleActorSerial.cpp)
add_library(example_actor_concurrent SHARED ExampleActorConcurrent.cpp)
add_library(example_worker_publish SHARED ExampleWorkerPublish.cpp)
add_library(example_worker_talk SHARED ExampleWorkerTalk.cpp)
add_library(example_worker_actor_interactive SHARED ExampleWorkerActorInteractive.cpp)

### install
FILE(GLOB conf_files "*.json")
INSTALL(FILES 
    ${conf_files}
    PERMISSIONS
    OWNER_READ
    GROUP_READ
    WORLD_READ
    DESTINATION ${MYFRAME_SERVICE_CONF_DIR}
)
INSTALL(TARGETS 
    example_actor_hello_world 
    example_actor_timer
    example_actor_serial
    example_actor_concurrent
    example_worker_actor_interactive 
    example_worker_publish
    example_worker_talk 
    LIBRARY DESTINATION ${MYFRAME_LIB_DIR}
    ARCHIVE DESTINATION ${MYFRAME_LIB_DIR}
    RUNTIME DESTINATION ${MYFRAME_BIN_DIR}
)