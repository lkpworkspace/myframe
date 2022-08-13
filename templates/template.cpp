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
    /* actor模块加载完毕后调用 */
    int Init(const char* param) override {
        /* 构造 hello,world 消息发送给自己 */
        return Send("actor.@template_name@.@template_name@1", std::make_shared<MyTextMsg>("hello,world"));
    }

    void CB(const std::shared_ptr<const MyMsg>& msg) override {
        if (msg->GetMsgType() == "TEXT") {
            /* 获得文本消息， 打印 源actor地址 目的actor地址 消息内容*/
            const auto& tmsg = std::dynamic_pointer_cast<const MyTextMsg>(msg);
            std::cout << "----> from \"" << tmsg->GetSrc() << "\" to \"" 
                << GetActorName() << "\": " << tmsg->GetData() << std::endl;
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

/* 创建actor实例函数 */
extern "C" std::shared_ptr<MyActor> my_actor_create(const std::string& actor_name) {
    return std::make_shared<@template_name@Actor>();
}

/* 创建worker实例函数 */
extern "C" MyWorker* my_worker_create(const std::string& worker_name) {
    return new @template_name@Worker();
}
