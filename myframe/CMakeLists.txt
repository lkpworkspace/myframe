cmake_minimum_required(VERSION 3.10)

### source
aux_source_directory(. __srcs)

### lib
add_library(${PROJECT_NAME} SHARED ${__srcs})
target_include_directories(${PROJECT_NAME}
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

### install
file(GLOB header_files
    macros.h
    common.h
    log.h
    msg.h
    mailbox.h
    cmd_channel.h
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
    DESTINATION lib
    EXPORT "${PROJECT_NAME}Targets"
)
