FROM docker.io/library/ubuntu:22.04
SHELL ["/bin/bash", "-c"]

ENV DEBIAN_FRONTEND noninteractive
COPY docker/ubuntu2204.sourcelist /etc/apt/sources.list
# RUN yes | unminimize
RUN apt update \
    && apt  install -y --no-install-recommends \
    build-essential cmake \
    wget python3 vim \
    libunwind-dev libgflags-dev

WORKDIR /tmp
RUN wget -q --content-disposition --no-check-certificate \
    https://github.com/open-source-parsers/jsoncpp/archive/refs/tags/1.9.5.tar.gz
RUN tar -xf jsoncpp-1.9.5.tar.gz \
 && cmake -S jsoncpp-1.9.5 -B build-jsoncpp \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DBUILD_OBJECT_LIBS=OFF \
    -DJSONCPP_WITH_TESTS=OFF \
    -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF \
    -DJSONCPP_WITH_PKGCONFIG_SUPPORT=OFF \
 && cmake --build build-jsoncpp --target install \
 && rm -rf jsoncpp-1.9.5.tar.gz jsoncpp-1.9.5 build-jsoncpp

RUN wget -q --content-disposition --no-check-certificate \
    https://github.com/google/glog/archive/refs/tags/v0.6.0.tar.gz
RUN tar -xf glog-0.6.0.tar.gz \
 && cmake -S glog-0.6.0 -B build-glog \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DWITH_PKGCONFIG=OFF \
    -DWITH_GTEST=OFF \
 && cmake --build build-glog --target install \
 && rm -rf glog-0.6.0.tar.gz glog-0.6.0 build-glog

ARG myframe_version=latest
RUN wget -q --content-disposition --no-check-certificate \
    https://github.com/lkpworkspace/myframe/archive/refs/tags/v${myframe_version}.tar.gz
RUN tar -xf myframe-${myframe_version}.tar.gz
RUN cmake -S myframe-${myframe_version} -B build-myframe \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
 && cmake --build build-myframe --target install \
 && rm -rf myframe-${myframe_version}.tar.gz myframe-${myframe_version} build-myframe
