#include <iostream>
#include <string.h>

#include "MyModule.h"
#include "MyMsg.h"

class @template_name@ : public MyModule
{
public:
    @template_name@(){}
    virtual ~@template_name@(){}

    /* 服务模块加载完毕后调用 */
    int Init(const char* param) override {
        /* 构造 hello,world 消息发送给自己 */
        MyTextMsg* msg = new MyTextMsg(GetHandle(),"hello,world");
        return Send(msg);
    }

    int CB(MyMsg* msg) override {
        MyTextMsg* tmsg = nullptr;
        switch(msg->GetMsgType()){
            case MyMsg::MyMsgType::TEXT:
                /* 获得文本消息， 打印 源服务地址 目的服务地址 消息内容*/
                tmsg = static_cast<MyTextMsg*>(msg);
                std::cout << "----> from \"" << tmsg->source << "\" to \"" 
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
extern "C" std::shared_ptr<MyModule> my_mod_create(const std::string& service_name) {
    return std::make_shared<@template_name@>();
}
