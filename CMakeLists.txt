cmake_minimum_required(VERSION 3.10)
project(myframe)

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
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
# glog option:
#   -DNDEBUG
#   -DDCHECK_ALWAYS_ON
# unit test option:
#   -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined
set(CMAKE_C_FLAGS
  "-fPIC -Wall -Wextra -Werror -pedantic-errors \
  -Wswitch-default -Wfloat-equal -Wshadow -Wcast-qual \
  -DNDEBUG"
)
set(CMAKE_CXX_FLAGS
  "-fPIC -Wall -Wextra -Werror -pedantic-errors \
  -Wswitch-default -Wfloat-equal -Wshadow -Wcast-qual \
  -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wsign-promo -Wsuggest-override \
  -DNDEBUG"
)
if (GCC_VERSION GREATER "8.0")
  list(APPEND CMAKE_CXX_FLAGS " -Wextra-semi ")
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
set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/${CMAKE_PROJECT_NAME}")
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(MYFRAME_TEST_DIR "test")
set(MYFRAME_BIN_DIR "bin")
set(MYFRAME_INC_DIR "inc")
set(MYFRAME_LIB_DIR "lib")
set(MYFRAME_LOG_DIR "log")
set(MYFRAME_SERVICE_DIR "service")
set(MYFRAME_CONF_DIR "conf")

### deps libs
find_package(jsoncpp REQUIRED)
find_package(gflags REQUIRED)
find_package(GTest REQUIRED)

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
    PERMISSIONS
    OWNER_READ
    GROUP_READ
    WORLD_READ
    DESTINATION .
)
install(DIRECTORY templates DESTINATION .)
install(DIRECTORY tools DESTINATION .)
install(DIRECTORY DESTINATION ${MYFRAME_LOG_DIR})
install(DIRECTORY DESTINATION ${MYFRAME_SERVICE_DIR})
