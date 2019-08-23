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

/*
    该服务实现：
        自己给自己发送一条消息
*/
class MyDemo : public MyModule
{
public:
    MyDemo(){}
    virtual ~MyDemo(){}

    virtual int Init(MyContext* c, const char* param) override
    {
        uint32_t handle = my_handle(c);
        std::cout << "MyDemo init" << std::endl;
        my_callback(c, CB, nullptr);
        const char* hello = "hello,world";
        my_send(c, 0, handle, handle, 0, (void*)hello, strlen(hello));
        return 0;
    }

    static int CB(MyContext* context, void *ud, int type, int session, uint32_t source , const void *msg, size_t sz)
    {
        std::string str((char*)msg,sz);
        std::cout << "----> from " << source << " to " << my_handle(context) << ": " << str << std::endl;
        return 0;
    }
};

extern "C" MyModule* my_mod_create()
{
    return static_cast<MyModule*>(new MyDemo());
}

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
