# myframe
![C++17](https://img.shields.io/badge/C++-17-blue)
![Linux Supported](https://img.shields.io/badge/Linux-Supported-brightgreen?logo=linux&logoColor=black)
![macOS Supported](https://img.shields.io/badge/macOS-Supported-brightgreen?logo=apple&logoColor=white)
![Windows Supported](https://img.shields.io/badge/Windows-Supported-brightgreen?logo=windows&logoColor=white)

[English](README_en.md) | 中文

## 概述
C++实现的组件化的编程框架,程序由actor和worker组成;  
actor基于消息驱动,actor之间可以进行消息传递;  
worker自驱动，可以通过消息与actor交互;  
适用于构建中大型项目.  

## github构建
* [github ci linux](.github/workflows/linux.yml)
* [github ci windows](.github/workflows/windows.yml)
* [github ci macOS](.github/workflows/macos.yml)

## 快速本地构建
```sh
# 安装依赖
cmake -S 3rd -B ".mypm/build_3rd" -DCMAKE_INSTALL_PREFIX=".mypm/install" -DMYFRAME_ENABLE_PYBIND=ON
cmake --build ".mypm/build_3rd" -j --config Release
# 构建安装
cmake --preset ReleaseLocal
cmake --build --preset ReleaseLocal
# 测试
ctest --preset ReleaseLocal
```

### Hello,World API示例
- [API 示例](test/test_hello.cpp)

### Hello,World 组件示例
- [组件代码示例](examples/example_actor_helloworld.cpp)
- [组件配置示例](examples/example_actor_helloworld.json)

## 程序接口
- [Example](examples)
- [Actor模块](myframe/actor.h)
- [Worker模块](myframe/worker.h)
- [Msg模块](myframe/msg.h)

## 文档
- [开发手册](doc/development_guide.md)
