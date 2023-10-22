cmake_minimum_required(VERSION 3.10)

### config
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

### source
aux_source_directory(. __srcs)

### lib
add_library(${PROJECT_NAME} SHARED ${__srcs})
target_include_directories(${PROJECT_NAME}
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# export file
include (GenerateExportHeader)
generate_export_header (${PROJECT_NAME}
  EXPORT_MACRO_NAME MYFRAME_EXPORT
  EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/export.h"
)

### install
file(GLOB header_files
    config.h
    platform.h
    export.h
    macros.h
    common.h
    log.h
    msg.h
    mailbox.h
    cmd_channel.h
    poller.h
    actor.h
    event.h
    worker.h
    mod_manager.h
    app.h
)
install(FILES 
    ${header_files}
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
    DESTINATION ${MYFRAME_INC_DIR}/${PROJECT_NAME}
)
install(TARGETS ${PROJECT_NAME}
    EXPORT "${PROJECT_NAME}Targets"
    LIBRARY DESTINATION ${MYFRAME_LIB_DIR}
    ARCHIVE DESTINATION ${MYFRAME_LIB_DIR}
    RUNTIME DESTINATION ${MYFRAME_BIN_DIR}
)
