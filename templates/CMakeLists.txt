cmake_minimum_required(VERSION 3.10)
project(@template_name@)

#### compile option
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#### deps lib
find_package(Threads REQUIRED)
find_package(jsoncpp REQUIRED) # need by myframe
find_package(glog REQUIRED) # need by myframe
find_package(myframe REQUIRED)

#### install path
set(MYFRAME_ROOT_DIR ${myframe_DIR}/../../..)

#### lib
add_library(${PROJECT_NAME} SHARED
    ${PROJECT_NAME}.cpp
)
target_link_libraries(${PROJECT_NAME}
    Threads::Threads
    myframe
)

#### install
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${MYFRAME_ROOT_DIR}/lib
    ARCHIVE DESTINATION ${MYFRAME_ROOT_DIR}/lib
    RUNTIME DESTINATION ${MYFRAME_ROOT_DIR}/bin
)
file(GLOB conf_files "*.json")
install(FILES
    ${conf_files}
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
    DESTINATION ${MYFRAME_ROOT_DIR}/service
)
