### configure file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/launcher_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/launcher_config.h @ONLY)

### source
# get_filename_component(DIR_NAME ${CMAKE_CURRENT_LIST_DIR} NAME_WE)
set(LAUNCHER_NAME "${PROJECT_NAME}_launcher")
aux_source_directory(. __srcs)

### bin
add_executable(${LAUNCHER_NAME} ${__srcs})
target_link_libraries(${LAUNCHER_NAME} PRIVATE ${PROJECT_NAME})

if (glog_lib_type STREQUAL "STATIC_LIBRARY")
  target_link_libraries(${LAUNCHER_NAME}
    PRIVATE
    glog::glog
  )
endif ()
if (jsoncpp_lib_type STREQUAL "STATIC_LIBRARY")
  target_link_libraries(${LAUNCHER_NAME}
    PRIVATE
    JsonCpp::JsonCpp
  )
endif ()

### install
install(TARGETS ${LAUNCHER_NAME}
  LIBRARY DESTINATION ${MYFRAME_LIB_DIR}
  ARCHIVE DESTINATION ${MYFRAME_LIB_DIR}
  RUNTIME DESTINATION ${MYFRAME_BIN_DIR}
)
install(DIRECTORY conf DESTINATION .)
