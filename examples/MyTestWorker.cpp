#include <string.h>
#include <iostream>
#include <thread>
#include <chrono>

#include <glog/logging.h>

#include "myframe/MyMsg.h"
#include "myframe/MyActor.h"
#include "myframe/MyWorker.h"

/// 回显worker发来的消息
class MyTestWorkerActor : public MyActor
{
public:
    int Init(const char* param) override {
    }

    void CB(const std::shared_ptr<const MyMsg>& msg) override {
        if (msg->GetMsgType() == "TEXT") {
            const auto& tmsg = std::dynamic_pointer_cast<const MyTextMsg>(msg);
            auto send_msg = std::make_shared<MyTextMsg>("this is actor msg from test worker actor");
            Send(msg->GetSrc(), send_msg);
        }
    }
};

/// 给MyTestWorkerActor发送消息，并打印收到的消息
class MyTestWorker : public MyWorker
{
public:
    MyTestWorker() {}
    virtual ~MyTestWorker() {}

    /// override MyWorker virtual method
    void Run() override {
        auto send_msg = std::make_shared<MyTextMsg>("this msg is from MyTestWorker");
        SendMsg("actor.testworker.#1", send_msg);
        DispatchAndWaitMsg();
        while (1) {
            const auto& msg = GetRecvMsg();
            if (msg == nullptr) {
                break;
            }
            const auto& tmsg = std::dynamic_pointer_cast<const MyTextMsg>(msg);
            LOG(INFO) << "get msg from " << tmsg->GetSrc() << " to " << tmsg->GetDst() << ": " << tmsg->GetData();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
};

/* 创建actor实例函数 */
extern "C" std::shared_ptr<MyActor> my_actor_create(const std::string& actor_name) {
    return std::make_shared<MyTestWorkerActor>();
}

/* 创建worker实例函数 */
extern "C" MyWorker* my_worker_create(const std::string& worker_name) {
    return new MyTestWorker();
}
