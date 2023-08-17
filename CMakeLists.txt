cmake_minimum_required(VERSION 3.10)
project(myframe VERSION 0.8.1)

### gcc version
if (CMAKE_COMPILER_IS_GNUCC)
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpfullversion -dumpversion
    OUTPUT_VARIABLE GCC_VERSION)
  string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})
  list(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)
  list(GET GCC_VERSION_COMPONENTS 1 GCC_MINOR)
  set(GCC_VERSION "${GCC_MAJOR}.${GCC_MINOR}")
endif ()

### cpp option
if (GCC_VERSION GREATER "8.0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra-semi")
endif ()

### option
option(GENERATE_EXAMPLE "generate example library" ON)
option(GENERATE_TEST "generate test exec" ON)

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
find_package(jsoncpp REQUIRED)
find_package(gflags REQUIRED)

link_libraries(
  pthread dl rt m
  glog gflags
  jsoncpp
  -Wl,-z,defs
)
if (GCC_VERSION LESS "8.0")
  link_libraries(
    stdc++fs
  )
endif ()

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
