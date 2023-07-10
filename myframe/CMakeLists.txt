cmake_minimum_required(VERSION 3.10)

### source
aux_source_directory(. __srcs)

### lib
add_library(${PROJECT_NAME} SHARED ${__srcs})

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
    mod_lib.h
    mod_manager.h
    app.h
)
install(FILES 
    ${header_files}
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
    DESTINATION ${MYFRAME_INC_DIR}/${PROJECT_NAME}
)
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${MYFRAME_LIB_DIR}
    ARCHIVE DESTINATION ${MYFRAME_LIB_DIR}
    RUNTIME DESTINATION ${MYFRAME_BIN_DIR}
)
