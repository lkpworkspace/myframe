cmake_minimum_required(VERSION 3.10)

### source
aux_source_directory(. MYFRAME_SRCS)
list(FILTER MYFRAME_SRCS EXCLUDE REGEX ".*main.cpp$")
set(MYFRAME_MAIN main.cpp)

### lib
add_library(myframe SHARED ${MYFRAME_SRCS})

### bin
add_executable(myframe_main ${MYFRAME_MAIN})
target_link_libraries(myframe_main myframe)

### install
FILE(GLOB header_files "*.h")
INSTALL(FILES 
    ${header_files}
    PERMISSIONS
    OWNER_READ
    GROUP_READ
    WORLD_READ
    DESTINATION ${CMAKE_INSTALL_PREFIX}/inc
)
install(TARGETS myframe
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

install(TARGETS myframe_main
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)