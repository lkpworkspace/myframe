cmake_minimum_required(VERSION 3.10)
# Allow the user to specify the MSVC runtime
if (POLICY CMP0091) # >= 3.15
  cmake_policy(SET CMP0091 NEW)
endif()
project(myframe
  VERSION 1.1.0
  LANGUAGES CXX
)

### options
option(MYFRAME_USE_CV "Using conditional variables for thread communication" ON)
option(MYFRAME_GENERATE_EXAMPLE "Generate example library" ON)
option(MYFRAME_GENERATE_TEST "Generate test executable program" ON)
option(MYFRAME_INSTALL_TEMPLATE "Install template project" ON)
option(MYFRAME_INSTALL_LAUNCHER "Install launcher program" ON)
option(MYFRAME_ENABLE_ASAN "Enable ASAN" OFF)
option(MYFRAME_ENABLE_PYBIND "Enable Python bindings" OFF)

### cmake module
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

### compile/link options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_options(
  # MSVC
  $<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>
  $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
  # GNU,Clang,AppleClang
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wextra>
  $<$<CXX_COMPILER_ID:GNU>:-Werror>
  $<$<CXX_COMPILER_ID:GNU>:-pedantic-errors>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wfloat-equal>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wshadow>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wcast-qual>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wnon-virtual-dtor>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wold-style-cast>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Woverloaded-virtual>
  $<$<CXX_COMPILER_ID:GNU>:-Wswitch-default>
  $<$<CXX_COMPILER_ID:GNU>:-Wsign-promo>
  $<$<CXX_COMPILER_ID:GNU>:-Wsuggest-override>
  $<$<CXX_COMPILER_ID:GNU>:-Wextra-semi>
)
if (MYFRAME_ENABLE_ASAN)
  add_compile_options(
    $<$<AND:$<CXX_COMPILER_ID:GNU,Clang,AppleClang>,$<CONFIG:Debug>>:-fsanitize=address,undefined>
    $<$<AND:$<CXX_COMPILER_ID:GNU,Clang,AppleClang>,$<CONFIG:Debug>>:-fno-omit-frame-pointer>
  )
  add_link_options(
    $<$<AND:$<CXX_COMPILER_ID:GNU,Clang,AppleClang>,$<CONFIG:Debug>>:-fsanitize=address,undefined>
    $<$<AND:$<CXX_COMPILER_ID:GNU,Clang,AppleClang>,$<CONFIG:Debug>>:-fno-omit-frame-pointer>
  )
endif ()

### install path
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  message(STATUS "Set default install prefix ${CMAKE_INSTALL_PREFIX}")
else()
  message(STATUS "Set install prefix ${CMAKE_INSTALL_PREFIX}")
endif()
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(MYFRAME_BIN_DIR "bin")
set(MYFRAME_INC_DIR "include")
set(MYFRAME_LIB_DIR "lib")
set(MYFRAME_LOG_DIR "log")
set(MYFRAME_SERVICE_DIR "service")
set(MYFRAME_CONF_DIR "conf")
set(MYFRAME_PYTHON_DIR "lib/python")

### deps libs
find_package(Threads REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(glog REQUIRED)
if (MYFRAME_ENABLE_PYBIND)
  find_package(SWIG REQUIRED)
  find_package(Python COMPONENTS Development REQUIRED)
endif()

get_target_property(glog_lib_type glog::glog TYPE)
get_target_property(jsoncpp_lib_type JsonCpp::JsonCpp TYPE)

### sub directory
add_subdirectory(myframe)
if (MYFRAME_INSTALL_LAUNCHER)
  add_subdirectory(launcher)
endif()
if (MYFRAME_GENERATE_EXAMPLE)
  add_subdirectory(examples)
endif()
if (MYFRAME_GENERATE_TEST)
  enable_testing()
  add_subdirectory(test)
endif()
if (MYFRAME_ENABLE_PYBIND)
  add_subdirectory(python)
endif()

### install file/dir
if (MYFRAME_INSTALL_TEMPLATE)
  install(DIRECTORY templates DESTINATION .)
endif()
install(PROGRAMS
  "tools/myframe_tool.py"
  "tools/myframe_setup.py"
  "tools/myframe_setup.sh"
  DESTINATION ${MYFRAME_BIN_DIR}
)
install(DIRECTORY DESTINATION ${MYFRAME_LOG_DIR})
install(DIRECTORY DESTINATION ${MYFRAME_SERVICE_DIR})

### package
include(Packing)
