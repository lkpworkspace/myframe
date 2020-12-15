# myframe

[![Build Status](https://travis-ci.com/lkpworkspace/myframe.svg?branch=master)](https://travis-ci.com/lkpworkspace/myframe)
[![cpp](https://img.shields.io/badge/language-cpp-green.svg)](https://img.shields.io/badge/language-cpp-green.svg)

## 概述
该项目是使用C++实现的actors框架,框架中每一个actor都可以称为一个服务或者是一个组件(这里都称为服务)，你可以将每一个独立的业务逻辑都编写成一个服务，服务之间可以进行消息传递，这样一个大型的应用程序或服务器程序就可以由多个服务组成，不同服务组合使用，既可以提高代码复用，又可以极大的降低程序耦合度，提高开发效率。

## 构建

```sh
mkdir build
cd build
cmake ../
make
```

## 运行

```sh
cd bin
./myframe --conf ../examples/config.json
```

## 服务 Hello,World Demo

```c
#include <iostream>
#include <string.h>

#include "MyModule.h"
#include "MyMsg.h"

/*
    该服务实现：
        自己给自己发送一条消息
*/
class MyDemo : public MyModule
{
public:
    MyDemo(){}
    virtual ~MyDemo(){}

    /* 服务模块加载完毕后调用 */
    virtual int Init(const char* param) override
    {
        /* 构造 hello,world 消息发送给自己 */
        MyTextMsg* msg = new MyTextMsg(GetHandle(),"hello,world");
        return Send(msg);
    }

    virtual int CB(MyMsg* msg) override
    {
        MyTextMsg* tmsg = nullptr;
        switch(msg->GetMsgType()){
            case MyMsg::MyMsgType::TEXT:
                /* 获得文本消息， 打印 源服务地址 目的服务地址 消息内容*/
                tmsg = static_cast<MyTextMsg*>(msg);
                std::cout << "----> from \"" << GetServiceName(tmsg->source) << "\" to \"" 
                    << GetServiceName() << "\": " << tmsg->GetData() << std::endl;
                break;
            default:
                /* 忽略其它消息 */
                std::cout << "Unknown msg type" << std::endl;
                break;
        }
        return 1;
    }
};

/* 创建服务模块实例函数 */
extern "C" MyModule* my_mod_create()
{
    return static_cast<MyModule*>(new MyDemo());
}

/* 销毁服务模块实例函数 */
extern "C" void my_mod_destory(MyModule* m)
{
    delete m;
}

```

## 服务配置文件
```json
{
    "thread":4,
    "module_path":"../CXXService/",
    "module_inst":{
        "demo":[
            {
                "name":"hello_world",
                "params":""
            }
        ]
    }
}

```
- thread:启动线程数
- module_path:模块目录
- module_inst:需要加载的模块
    - demo：加载模块名
        - name：使用该模块生成的服务名
        - params：传递给该服务的参数

## 程序接口

- [服务模块](https://github.com/lkpworkspace/myframe/blob/master/myframe/MyModule.h)

- [消息类型](https://github.com/lkpworkspace/myframe/blob/master/myframe/MyMsg.h)

## 文档
- [文档入口](https://github.com/lkpworkspace/myframe/wiki)

## 常见问题
- [FAQs](https://github.com/lkpworkspace/myframe/wiki/FAQs)
