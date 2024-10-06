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
cd 3rd
cmake -S . -B build -DCMAKE_INSTALL_PREFIX="../output"
cmake --build build -j --config Release
# 构建安装
cd ..
cmake -S . -B build -DCMAKE_INSTALL_PREFIX="./output" -DCMAKE_PREFIX_PATH="./output"
cmake --build build -j --config Release --target install
```

### Hello,World 示例
```c
#include <string.h>
#include <iostream>

#include "myframe/msg.h"
#include "myframe/actor.h"

using namespace myframe;
/*
  该actor实现：
    自己给自己发送一条消息
*/
class Demo : public Actor
{
 public:
  /* actor模块加载完毕后调用 */
  int Init(const char* param) override {
    /* 构造 hello,world 消息发送给自己 */
    auto mailbox = GetMailbox();
    mailbox->Send("actor.demo.echo_hello_world", std::make_shared<Msg>("hello,world"));
  }

  void Proc(const std::shared_ptr<const Msg>& msg) override {
    /* 获得文本消息， 打印 源actor地址 目的actor地址 消息内容 */
    std::cout << *msg << ": " << msg->GetData() << std::endl;
  }
};

/* 框架根据描述文件创建actor实例函数 */
extern "C" std::shared_ptr<Actor> actor_create(const std::string& actor_name) {
  if (actor_name == "demo") {
    return std::make_shared<Demo>();
  }
  return nullptr;
}

```

### Hello,World 配置文件
```json
{
  "type":"library",
  "lib":"demo",
  "actor":{
    "demo":[
      {
        "instance_name":"echo_hello_world",
        "instance_params":""
      }
    ]
  }
}
```
- type: [ library | class ]
- lib: 库名称
- actor: 需要创建的actor列表
  - demo: actor名
    - instance_name：实例名称
    - instance_params：实例参数

## 程序接口
- [Example](https://github.com/lkpworkspace/myframe/tree/master/examples)
- [Actor模块](https://github.com/lkpworkspace/myframe/blob/master/myframe/actor.h)
- [Worker模块](https://github.com/lkpworkspace/myframe/blob/master/myframe/worker.h)
- [Msg模块](https://github.com/lkpworkspace/myframe/blob/master/myframe/msg.h)

## 文档
- [开发手册](doc/development_guide.md)
- [Discussions](https://github.com/lkpworkspace/myframe/discussions)
- [WIKI](https://github.com/lkpworkspace/myframe/wiki)
- [FAQ](https://github.com/lkpworkspace/myframe/wiki/FAQs)
