cmake_minimum_required(VERSION 3.10)

### configure file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/flags.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/flags.cpp @ONLY)

### source
aux_source_directory(. MYFRAME_SRCS)
list(FILTER MYFRAME_SRCS EXCLUDE REGEX ".*main.cpp$")
set(MYFRAME_MAIN main.cpp)

### lib
add_library(myframe SHARED ${MYFRAME_SRCS})

### bin
add_executable(myframe_main ${MYFRAME_MAIN})
target_link_libraries(myframe_main myframe)

### install
file(GLOB header_files 
    log.h
    msg.h
    actor.h
    event.h
    worker.h
    mod_lib.h
    mod_manager.h
    app.h
    flags.h
)
install(FILES 
    ${header_files}
    PERMISSIONS
    OWNER_READ
    GROUP_READ
    WORLD_READ
    DESTINATION ${MYFRAME_INC_DIR}/myframe
)
install(TARGETS myframe myframe_main
    LIBRARY DESTINATION ${MYFRAME_LIB_DIR}
    ARCHIVE DESTINATION ${MYFRAME_LIB_DIR}
    RUNTIME DESTINATION ${MYFRAME_BIN_DIR}
)