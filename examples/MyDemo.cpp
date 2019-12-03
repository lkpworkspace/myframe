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
