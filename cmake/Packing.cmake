set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR ${PROJECT_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ application framework")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
set(CPACK_STRIP_FILES YES)
set(CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT "likepeng0418@163.com")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
# package only the server runtime so dependency headers and static libs stay out of the installer
# set(CPACK_COMPONENTS_ALL runtime)

# group
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

if (WIN32)
    # Download link for the portable binary zip (no admin needed):
    #   https://github.com/wixtoolset/wix3/releases/download/wix314rtm/wix314-binaries.zip
    # 添加path/to/wix/v3.14/bin路径到环境变量
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION bin)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT runtime)
    include(InstallRequiredSystemLibraries)
    set(CPACK_GENERATOR "WIX")
    # a stable upgrade guid lets new installers replace older ones instead of installing side by side
    set(CPACK_WIX_UPGRADE_GUID "EEFDB893-E848-4637-B2D9-BEEAEF4C00C0")
    set(CPACK_WIX_PROPERTY_ARPHELPLINK "https://github.com/lkpworkspace/myframe")
    set(CPACK_WIX_ROOT_FEATURE_TITLE ${PROJECT_NAME})
    # add the install bin directory to the system path
    # set(CPACK_WIX_PATCH_FILE "${CMAKE_SOURCE_DIR}/cmake/wix-patch.xml")
elseif (APPLE)
    install(FILES "${CMAKE_SOURCE_DIR}/LICENSE.txt" DESTINATION share/doc/myframe RENAME LICENSE.txt COMPONENT runtime)
    set(CPACK_GENERATOR "productbuild")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
else ()
    install(FILES "${CMAKE_SOURCE_DIR}/LICENSE.txt" DESTINATION share/doc/myframe RENAME copyright COMPONENT runtime)
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "likepeng <${CPACK_PACKAGE_CONTACT}>")
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEB_COMPONENT_INSTALL YES)
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS YES)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
endif ()

include(CPack)
