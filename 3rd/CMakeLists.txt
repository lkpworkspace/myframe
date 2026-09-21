cmake_minimum_required(VERSION 3.10)
# Allow the user to specify the MSVC runtime
if (POLICY CMP0091) # >= 3.15
  cmake_policy(SET CMP0091 NEW)
endif()
# Disable DOWNLOAD_EXTRACT_TIMESTAMP warning
if (POLICY CMP0135) # >= 3.24
  cmake_policy(SET CMP0135 OLD)
endif()
project(myframe_deps VERSION 1.0.0)

include(ExternalProject)

option(MYFRAME_ENABLE_PYBIND "Enable Python bindings" OFF)

set(DEPS_SOURCE_DIR ${CMAKE_BINARY_DIR}/src CACHE PATH "package source dir")
set(DEPS_DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/pkg CACHE PATH "package tgz dir")

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
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/gflags-2.2.2
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
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
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/glog-0.6.0
        -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}/gflags-2.2.2
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
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
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/jsoncpp-1.9.5
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
)

if (MYFRAME_ENABLE_PYBIND)
    ExternalProject_Add(
        swig
        URL https://github.com/swig/swig/archive/refs/tags/v4.3.1.tar.gz
        URL_MD5 ${swig_MD5}
        DOWNLOAD_NAME "swig.tar.gz"
        PREFIX ${CMAKE_BINARY_DIR}
        DOWNLOAD_DIR ${DEPS_DOWNLOAD_DIR}
        SOURCE_DIR "${DEPS_SOURCE_DIR}/swig"
        CMAKE_ARGS
            -DBUILD_SHARED_LIBS=ON
            -DCMAKE_BUILD_TYPE=Release
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}/swig-4.3.1
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    )
endif()
