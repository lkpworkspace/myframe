# myframe

![myframe](doc/pics/myframe.png)

## 概述
C++实现的组件化的编程框架,程序由actor和worker组成;  
actor基于消息驱动,actor之间可以进行消息传递;  
worker自驱动，可以通过消息与actor交互;  
适用于构建中大型项目.  

## 开发/运行环境
| C++ 标准支持   |
| -------------- |
| C++17          |
| C++20          |

| 操作系统支持   |
| -------------- |
| Linux |
| Windows |
| macOS |

## github构建
* [github ci linux](.github/workflows/linux.yml)
* [github ci windows](.github/workflows/windows.yml)
* [github ci macOS](.github/workflows/macos.yml)

## 快速本地构建
```sh
# 下载/构建/安装依赖库
cmake -S 3rd -B build_3rd -DCMAKE_INSTALL_PREFIX="./output"
cmake --build build_3rd -j --config Release
# 构建安装
cmake -S . -B build_proj -DCMAKE_INSTALL_PREFIX="./output" -DCMAKE_PREFIX_PATH="./output"
cmake --build build_proj -j --config Release --target install
```

### Hello,World API示例
- [API 示例](test/hello_test.cpp)

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
- [Discussions](https://github.com/lkpworkspace/myframe/discussions)
- [WIKI](https://github.com/lkpworkspace/myframe/wiki)
- [FAQ](https://github.com/lkpworkspace/myframe/wiki/FAQs)
