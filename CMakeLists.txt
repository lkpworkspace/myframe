cmake_minimum_required(VERSION 3.10)
project(myframe VERSION 0.9.2)

### option
option(MYFRAME_USE_CV "Using conditional variables for thread communication" ON)
option(MYFRAME_INSTALL_DEPS "Install deps" OFF)
option(MYFRAME_GENERATE_EXAMPLE "Generate example library" ON)
option(MYFRAME_GENERATE_TEST "Generate test executable program" ON)

### cmake module
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

### compile option
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
if (CMAKE_CXX_STANDARD_REQUIRED)
  message(STATUS "Set cxx standard ${CMAKE_CXX_STANDARD}")
else()
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_STANDARD 17)
  message(STATUS "Set default cxx standard 17")
endif()

### install path
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  message(STATUS "Set default install prefix ${CMAKE_INSTALL_PREFIX}")
else()
  message(STATUS "Set install prefix ${CMAKE_INSTALL_PREFIX}")
endif()
set(MYFRAME_BIN_DIR "bin")
set(MYFRAME_INC_DIR "include")
set(MYFRAME_LIB_DIR "lib")
set(MYFRAME_LOG_DIR "log")
set(MYFRAME_SERVICE_DIR "service")
set(MYFRAME_CONF_DIR "conf")

### deps libs
find_package(Threads REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(glog REQUIRED)

link_libraries(
  ${CMAKE_DL_LIBS}
  Threads::Threads
  glog::glog
  jsoncpp_lib
)

### include dir
include_directories(.)

### sub directory
add_subdirectory(myframe)
add_subdirectory(launcher)
if (MYFRAME_GENERATE_EXAMPLE)
  add_subdirectory(examples)
endif()
if (MYFRAME_GENERATE_TEST)
  add_subdirectory(test)
endif()

### install file/dir
install(FILES
  "LICENSE"
  PERMISSIONS
  OWNER_READ OWNER_WRITE
  GROUP_READ
  WORLD_READ
  DESTINATION .
)
install(PROGRAMS
  "tools/gen_mod_proj.py"
  DESTINATION ${MYFRAME_BIN_DIR}
)
install(DIRECTORY templates DESTINATION .)
install(DIRECTORY DESTINATION ${MYFRAME_LOG_DIR})
install(DIRECTORY DESTINATION ${MYFRAME_SERVICE_DIR})
install(CODE "
  file(
    WRITE ${CMAKE_INSTALL_PREFIX}/version.txt
    \"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}\"
  )
")
if (MYFRAME_INSTALL_DEPS)
if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.21")
  install(IMPORTED_RUNTIME_ARTIFACTS jsoncpp_lib)
  install(IMPORTED_RUNTIME_ARTIFACTS glog::glog)
  install(IMPORTED_RUNTIME_ARTIFACTS gflags)
else()
  message(WARNING "Can not install deps lib, cmake version ${CMAKE_VERSION} < 3.21")
endif()
endif()

### package
include(Packing)
