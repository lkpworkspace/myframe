#include <iostream>
#include <string.h>

#include "myframe/MyModule.h"
#include "myframe/MyMsg.h"

/*
    该服务实现：
        自己给自己发送一条消息
*/
class MyDemo : public MyModule
{
public:
    /* 服务模块加载完毕后调用 */
    int Init(const char* param) override {
        /* 构造 hello,world 消息发送给自己 */
        auto msg = std::make_shared<MyTextMsg>(GetHandle(),"hello,world");
        return Send(msg);
    }

    void CB(std::shared_ptr<MyMsg>& msg) override {
        switch(msg->GetMsgType()){
            case MyMsg::MyMsgType::TEXT: {
                /* 获得文本消息， 打印 源服务地址 目的服务地址 消息内容*/
                auto tmsg = std::dynamic_pointer_cast<MyTextMsg>(msg);
                std::cout << "----> from \"" << tmsg->source << "\" to \"" 
                    << GetServiceName() << "\": " << tmsg->GetData() << std::endl;
                break;
            }
            default:
                /* 忽略其它消息 */
                std::cout << "Unknown msg type" << std::endl;
                break;
        }
    }
};

/* 创建服务模块实例函数 */
extern "C" std::shared_ptr<MyModule> my_mod_create(const std::string& service_name) {
    return std::make_shared<MyDemo>();
}
