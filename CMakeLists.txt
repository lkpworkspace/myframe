cmake_minimum_required(VERSION 3.10)
project(myframe)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
# -DNDEBUG -DDCHECK_ALWAYS_ON=0
set(CMAKE_C_FLGAS "-fPIC -DPIC -DNDEBUG -DDCHECK_ALWAYS_ON=0")
set(CMAKE_CXX_FLGAS "-fPIC -DPIC -DNDEBUG -DDCHECK_ALWAYS_ON=0") 
add_compile_options(-Werror=return-type)

option(GENERATE_EXAMPLE "generate example library" ON)
option(GENERATE_TEST "generate test exec" ON)

### output path
set(MYFRAME_OUTPUT_ROOT "${CMAKE_BINARY_DIR}/output")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MYFRAME_OUTPUT_ROOT}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MYFRAME_OUTPUT_ROOT}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MYFRAME_OUTPUT_ROOT}/bin)

### install path
set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/${CMAKE_PROJECT_NAME}")
set(MYFRAME_TEST_DIR "${CMAKE_INSTALL_PREFIX}/test/")
set(MYFRAME_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin/")
set(MYFRAME_INC_DIR "${CMAKE_INSTALL_PREFIX}/inc/")
set(MYFRAME_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/")
set(MYFRAME_LOG_DIR "${CMAKE_INSTALL_PREFIX}/log/")
set(MYFRAME_SERVICE_DIR "${CMAKE_INSTALL_PREFIX}/service/")
set(MYFRAME_CONF_DIR "${CMAKE_INSTALL_PREFIX}/conf/")

### deps libs
find_package(jsoncpp REQUIRED)
find_package(gflags REQUIRED)
find_package(GTest REQUIRED)

link_libraries(pthread dl rt m)
link_libraries(glog gflags jsoncpp)

### include dir
include_directories(.)

### sub directory
add_subdirectory(myframe)
add_subdirectory(misc)
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
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)
install(DIRECTORY templates DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY tools DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY DESTINATION ${MYFRAME_LOG_DIR})
install(DIRECTORY DESTINATION ${MYFRAME_SERVICE_DIR})