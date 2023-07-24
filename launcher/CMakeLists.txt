CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

### configure file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/launcher_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/launcher_config.h @ONLY)

### source
get_filename_component(DIR_NAME ${CMAKE_CURRENT_LIST_DIR} NAME_WE)
aux_source_directory(. __srcs)

### bin
add_executable(${DIR_NAME} ${__srcs})
target_link_libraries(${DIR_NAME} ${PROJECT_NAME})

### install
install(TARGETS ${DIR_NAME}
    LIBRARY DESTINATION ${MYFRAME_LIB_DIR}
    ARCHIVE DESTINATION ${MYFRAME_LIB_DIR}
    RUNTIME DESTINATION ${MYFRAME_BIN_DIR}
)
