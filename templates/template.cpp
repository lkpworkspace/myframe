#include <string.h>
#include <iostream>
#include <thread>
#include <chrono>

#include "MyMsg.h"
#include "MyActor.h"
#include "MyWorker.h"

class @template_name@Actor : public MyActor
{
public:
    @template_name@Actor(){}
    virtual ~@template_name@Actor(){}

    /* 服务模块加载完毕后调用 */
    int Init(const char* param) override {
        /* 构造 hello,world 消息发送给自己 */
        auto msg = std::make_shared<MyTextMsg>(GetHandle(),"demo: hello,world");
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

class @template_name@Worker : public MyWorker
{
public:
    @template_name@Worker() {}
    virtual ~@template_name@Worker() {}

    /// override MyWorker virtual method
    void Run() override {
        DispatchMsg();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << "hello, wolrd" << std::endl;
    }
};

/* 创建服务实例函数 */
extern "C" std::shared_ptr<MyActor> my_actor_create(const std::string& service_name) {
    return std::make_shared<@template_name@Actor>();
}

/* 创建worker实例函数 */
extern "C" MyWorker* my_worker_create(const std::string& worker_name) {
    return new @template_name@Worker();
}
