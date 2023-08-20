cmake_minimum_required(VERSION 3.10)
project(myframe VERSION 0.8.2)

### option
option(GENERATE_EXAMPLE "generate example library" ON)
option(GENERATE_TEST "generate test exec" ON)

### compile option
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

### output path
set(MYFRAME_OUTPUT_ROOT "${CMAKE_BINARY_DIR}/output")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MYFRAME_OUTPUT_ROOT}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MYFRAME_OUTPUT_ROOT}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MYFRAME_OUTPUT_ROOT}/bin)

### install path
# set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/${CMAKE_PROJECT_NAME}")
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(MYFRAME_TEST_DIR "test")
set(MYFRAME_BIN_DIR "bin")
set(MYFRAME_INC_DIR "include")
set(MYFRAME_LIB_DIR "lib")
set(MYFRAME_LOG_DIR "log")
set(MYFRAME_SERVICE_DIR "service")
set(MYFRAME_CONF_DIR "conf")

### deps libs
find_package(Threads REQUIRED)
find_package(jsoncpp REQUIRED)

link_libraries(
  Threads::Threads
  ${CMAKE_DL_LIBS}
  glog
  jsoncpp
)

### include dir
include_directories(.)

### sub directory
add_subdirectory(myframe)
add_subdirectory(launcher)
if (GENERATE_EXAMPLE)
    add_subdirectory(examples)
endif()
if (GENERATE_TEST)
    add_subdirectory(test)
endif()

### install file/dir
install(FILES
    "LICENSE"
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
    DESTINATION .
)
install(DIRECTORY templates DESTINATION .)
install(DIRECTORY tools DESTINATION .)
install(DIRECTORY conf DESTINATION .)
install(DIRECTORY DESTINATION ${MYFRAME_LOG_DIR})
install(DIRECTORY DESTINATION ${MYFRAME_SERVICE_DIR})
install(CODE "
  file(
    WRITE ${CMAKE_INSTALL_PREFIX}/version.txt
    \"${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}\"
  )
")

### package
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${${PROJECT_NAME}_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${${PROJECT_NAME}_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${${PROJECT_NAME}_VERSION_PATCH}")
include(CPack)

### export cmake file
install(EXPORT "${PROJECT_NAME}Targets"
  FILE "${PROJECT_NAME}Targets.cmake"
  DESTINATION lib/cmake/${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)
# generate the config file that is includes the exports
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION "lib/cmake/${PROJECT_NAME}"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# generate the version file for the config file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}"
  COMPATIBILITY AnyNewerVersion
)

# install the configuration file
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION lib/cmake/${PROJECT_NAME}
)

# generate the export targets for the build tree
# needs to be after the install(TARGETS ) command
export(EXPORT "${PROJECT_NAME}Targets"
  FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake"
)
