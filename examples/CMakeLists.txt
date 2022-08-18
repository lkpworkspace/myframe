CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

### Actor
add_library(demo SHARED MyDemo.cpp)
add_library(test_timer SHARED MyTestTimer.cpp)
add_library(test_worker SHARED MyTestWorker.cpp)

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
INSTALL(TARGETS demo test_timer test_worker
    LIBRARY DESTINATION ${MYFRAME_LIB_DIR}
    ARCHIVE DESTINATION ${MYFRAME_LIB_DIR}
    RUNTIME DESTINATION ${MYFRAME_BIN_DIR}
)