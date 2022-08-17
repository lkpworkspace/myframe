cmake_minimum_required(VERSION 3.10)
project(myframe)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_FLGAS "-fPIC -DPIC")
set(CMAKE_CXX_FLGAS "-fPIC -DPIC") # -DNDEBUG

### output path
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/output/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/output/bin")

### install path
set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/${CMAKE_PROJECT_NAME}")
set(MYFRAME_INC_DIR "${CMAKE_INSTALL_PREFIX}/inc/")
set(MYFRAME_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/")
set(MYFRAME_LOG_DIR "${CMAKE_INSTALL_PREFIX}/log/")
set(MYFRAME_SERVICE_CONF_DIR "${CMAKE_INSTALL_PREFIX}/service/")

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
add_subdirectory(examples)
add_subdirectory(test)
add_subdirectory(misc)

### install file/dir
INSTALL(FILES 
    "LICENSE"
    PERMISSIONS
    OWNER_READ
    GROUP_READ
    WORLD_READ
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)
install(DIRECTORY templates DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY tools DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/log)
install(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/service)