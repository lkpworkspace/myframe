cmake_minimum_required(VERSION 3.10)
project(@template_name@ VERSION 1.0.0)

#### compile option
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#### cmake module
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#### deps lib
find_package(myframe REQUIRED)

#### lib
add_library(${PROJECT_NAME} SHARED
  ${PROJECT_NAME}.cpp
)
target_link_libraries(${PROJECT_NAME}
  myframe
)
target_compile_definitions(${PROJECT_NAME} PRIVATE myframe_EXPORTS)

#### install
install(TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)
file(GLOB conf_files "*.json")
install(FILES
  ${conf_files}
  PERMISSIONS
  OWNER_READ OWNER_WRITE
  GROUP_READ
  WORLD_READ
  DESTINATION ${CMAKE_INSTALL_PREFIX}/service
)

### package
include(Packing)
