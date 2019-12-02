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

        /* 构造 hello,world 消息发送给自己 */
        MyTextMsg* msg = new MyTextMsg(m_handle,"hello,world");
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
