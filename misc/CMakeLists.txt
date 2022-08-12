cmake_minimum_required(VERSION 3.10)

FILE(GLOB scripts_files "*.bash")
INSTALL(FILES 
    ${scripts_files}
    PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
    WORLD_READ
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)