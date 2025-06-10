cmake_minimum_required(VERSION 3.10)

### config
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

### source
aux_source_directory(. __srcs)

### lib
add_library(${PROJECT_NAME} ${__srcs})
target_include_directories(${PROJECT_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
  INTERFACE
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(${PROJECT_NAME}
  PRIVATE
  ${CMAKE_DL_LIBS}
  PUBLIC
  Threads::Threads
)

if (glog_lib_type STREQUAL "STATIC_LIBRARY")
  target_link_libraries(${PROJECT_NAME}
    PRIVATE
    glog::glog
  )
else ()
  set(glog_DEPENDENCY "find_dependency (glog REQUIRED)")
  target_link_libraries(${PROJECT_NAME}
    PUBLIC
    glog::glog
  )
endif ()
if (jsoncpp_lib_type STREQUAL "STATIC_LIBRARY")
  target_link_libraries(${PROJECT_NAME}
    PRIVATE
    JsonCpp::JsonCpp
  )
else ()
  set(jsoncpp_DEPENDENCY "find_dependency (jsoncpp REQUIRED)")
  target_link_libraries(${PROJECT_NAME}
    PUBLIC
    JsonCpp::JsonCpp
  )
endif ()

if (glog_lib_type STREQUAL "STATIC_LIBRARY" OR jsoncpp_lib_type STREQUAL "STATIC_LIBRARY")
  get_target_property(${PROJECT_NAME}_lib_type ${PROJECT_NAME} TYPE)
  if (${PROJECT_NAME}_lib_type STREQUAL "SHARED_LIBRARY")
    target_link_options(${PROJECT_NAME}
      PRIVATE
      "$<$<CXX_COMPILER_ID:GNU,Clang>:LINKER:--exclude-libs,ALL>"
    )
  endif ()
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

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
  shared_library.h
  app.h
)
install(FILES 
  ${header_files}
  PERMISSIONS
  OWNER_READ OWNER_WRITE
  GROUP_READ
  WORLD_READ
  DESTINATION ${MYFRAME_INC_DIR}/${PROJECT_NAME}
)
install(TARGETS ${PROJECT_NAME}
  EXPORT "${PROJECT_NAME}Targets"
  LIBRARY DESTINATION ${MYFRAME_LIB_DIR}
  ARCHIVE DESTINATION ${MYFRAME_LIB_DIR}
  RUNTIME DESTINATION ${MYFRAME_BIN_DIR}
)

include(InstallingConfigs)
