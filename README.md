# ![run](https://github.com/lkpworkspace/demo/blob/master/pics/icon3.png)MyFrame

MyFrame is a lightweight event-driven application framework

## 概述
	- 使用线程池实现的程序框架
    - 基于服务的编程模式, 服务之间使用消息进行通信
	- 支持服务间无锁通信，提高并发性能
    - 仅支持linux
	- No document (you can find more docs in the code)
	
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
#include "MyFrame.h"
#include "MyContext.h"
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
    virtual int Init(MyContext* c, const char* param) override
    {
        /* Demo服务的句柄号 */
        m_handle = my_handle(c);

        /* 设置处理消息的回调函数 */
        my_callback(c, CB, this);

        /* 构造一条文本消息发送给自己 */
        MyTextMsg* msg = new MyTextMsg();
        msg->source = m_handle;               // 源地址: Demo服务
        msg->destination = m_handle;          // 目的地址: Demo服务
        std::string s("hello,world");         // 内容: hello,world 字符串
        msg->SetData(s);

        return my_send(c, msg);
    }

    /* 服务消息处理函数 */
    static int CB(MyContext* context, MyMsg* msg, void* ud)
    {
        /* Demo服务对象 */
        MyDemo* self = static_cast<MyDemo*>(ud);

        MyTextMsg* tmsg = nullptr;
        switch(msg->GetMsgType()){
            case MyMsg::MyMsgType::TEXT:
                /* 获得文本消息， 打印 源服务地址 目的服务地址 消息内容*/
                tmsg = static_cast<MyTextMsg*>(msg);
                std::cout << "----> from " << tmsg->source << " to " 
                    << self->m_handle << ": " << tmsg->GetData() << std::endl;
                break;
            default:
                /* 忽略其它消息 */
                std::cout << "Unknown msg type" << std::endl;
                break;
        }
        return 0;
    }

    uint32_t m_handle;
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
        "libdemo.so":""
    }
}

```
- thread:
    - 启动线程数
- module_path:
    - 模块目录
- module_inst:
    - 实例化的模块名:传递给实例对象参数

## 调试日志
* doc/debug.md
