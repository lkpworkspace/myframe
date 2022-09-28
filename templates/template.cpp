#include <thread>
#include <chrono>
#include <memory>

#include <glog/logging.h>

#include "MyMsg.h"
#include "MyActor.h"
#include "MyWorker.h"

using namespace myframe;

class @template_name@Actor : public MyActor
{
public:
    /* actor模块加载完毕后调用 */
    int Init(const char* param) override {
        return 0;
    }

    void Proc(const std::shared_ptr<const MyMsg>& msg) override {
        if (msg->GetMsgType() == "TEXT") {
            /* 获得文本消息， 打印 源地址 目的地址 消息内容*/
            LOG(INFO) << "get msg from \"" << msg->GetSrc() << ": " << msg->GetData();
            /* 回复消息 */
            Send(msg->GetSrc(), std::make_shared<MyMsg>("this is actor resp"));
        }
    }
};

class @template_name@Worker : public MyWorker
{
public:
    @template_name@Worker() {}
    virtual ~@template_name@Worker() {}

    /* myframe会循环调用该函数 */
    void Run() override {
        /* 给 actor.@template_name@ 发送消息，并接收回复消息 */
        auto send_msg = std::make_shared<MyMsg>("this is template worker req");
        SendMsg("actor.@template_name@.@template_name@1", send_msg);
        DispatchAndWaitMsg();
        while (1) {
            const auto& msg = GetRecvMsg();
            if (msg == nullptr) {
                break;
            }
            LOG(INFO) << "get msg from " << msg->GetSrc() << ": " << msg->GetData();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
};

/* 创建actor实例函数 */
extern "C" std::shared_ptr<MyActor> my_actor_create(const std::string& actor_name) {
    return std::make_shared<@template_name@Actor>();
}

/* 创建worker实例函数 */
extern "C" std::shared_ptr<MyWorker> my_worker_create(const std::string& worker_name) {
    return std::make_shared<@template_name@Worker>();
}
