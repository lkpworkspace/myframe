cmake_minimum_required(VERSION 3.10)
if (POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()
project(myframe_deps VERSION 1.0.0)

include(ExternalProject)

set(DEPS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(DEPS_DOWNLOAD_DIR ${CMAKE_SOURCE_DIR}/pkg)

ExternalProject_Add(
    gflags
    URL https://github.com/gflags/gflags/archive/refs/tags/v2.2.2.tar.gz
    URL_MD5 1a865b93bacfa963201af3f75b7bd64c
    DOWNLOAD_NAME "gflags.tar.gz"
    PREFIX ${CMAKE_BINARY_DIR}
    DOWNLOAD_DIR ${DEPS_DOWNLOAD_DIR}
    SOURCE_DIR "${DEPS_SOURCE_DIR}/gflags"
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CMAKE_ARGS
        -DBUILD_SHARED_LIBS=ON
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
)

ExternalProject_Add(
    glog
    URL https://github.com/google/glog/archive/refs/tags/v0.6.0.tar.gz
    URL_MD5 c98a6068bc9b8ad9cebaca625ca73aa2
    DOWNLOAD_NAME "glog.tar.gz"
    PREFIX ${CMAKE_BINARY_DIR}
    DOWNLOAD_DIR ${DEPS_DOWNLOAD_DIR}
    SOURCE_DIR "${DEPS_SOURCE_DIR}/glog"
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CMAKE_ARGS
        -DBUILD_SHARED_LIBS=ON
        -DCMAKE_BUILD_TYPE=Release
        -DWITH_PKGCONFIG=OFF
        -DWITH_GTEST=OFF
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
)
ExternalProject_Add_StepDependencies(glog install gflags)

ExternalProject_Add(
    jsoncpp
    URL https://github.com/open-source-parsers/jsoncpp/archive/refs/tags/1.9.5.tar.gz
    URL_MD5 d6c8c609f2162eff373db62b90a051c7
    DOWNLOAD_NAME "jsoncpp.tar.gz"
    PREFIX ${CMAKE_BINARY_DIR}
    DOWNLOAD_DIR ${DEPS_DOWNLOAD_DIR}
    SOURCE_DIR "${DEPS_SOURCE_DIR}/jsoncpp"
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CMAKE_ARGS
        -DBUILD_SHARED_LIBS=ON
        -DBUILD_STATIC_LIBS=OFF
        -DBUILD_OBJECT_LIBS=OFF
        -DCMAKE_BUILD_TYPE=Release
        -DJSONCPP_WITH_TESTS=OFF
        -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF
        -DJSONCPP_WITH_PKGCONFIG_SUPPORT=OFF
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
)
