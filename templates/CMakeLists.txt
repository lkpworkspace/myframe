cmake_minimum_required(VERSION 3.10)
project(@template_name@)

#### compile option
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#### path setting
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(MYFRAME_INSTALL_SERVICE_DIR ${CMAKE_INSTALL_PREFIX}/service)
set(MYFRAME_INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib)

#### dep 3rd lib
find_package(Threads REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(glog REQUIRED)

#### include directory
include_directories(${CMAKE_INSTALL_PREFIX}/include)

#### deps libs
link_libraries(
    Threads::Threads
    glog::glog
    jsoncpp_lib
)
link_directories(${MYFRAME_INSTALL_LIB_DIR})

#### lib
add_library(${PROJECT_NAME} SHARED
    ${PROJECT_NAME}.cpp
)
target_link_libraries(${PROJECT_NAME}
    myframe
)

#### install
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${MYFRAME_INSTALL_LIB_DIR}
    ARCHIVE DESTINATION ${MYFRAME_INSTALL_LIB_DIR}
    RUNTIME DESTINATION ${MYFRAME_INSTALL_LIB_DIR}
)
file(GLOB conf_files "*.json")
install(FILES
    ${conf_files}
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
    DESTINATION ${MYFRAME_INSTALL_SERVICE_DIR}
)
