#include <thread>
#include <chrono>

#include <glog/logging.h>

#include "myframe/MyMsg.h"
#include "myframe/MyActor.h"
#include "myframe/MyWorker.h"

using namespace myframe;

/// 回显worker发来的消息
class ExampleActorInteractive : public MyActor
{
public:
    int Init(const char* param) override {
        return 0;
    }

    void Proc(const std::shared_ptr<const MyMsg>& msg) override {
        if (msg->GetMsgType() == "TEXT") {
            Send(msg->GetSrc(), std::make_shared<MyMsg>("this is ExampleActorInteractive resp"));
        }
    }
};

/// 给MyTestWorkerActor发送消息，并打印收到的消息
class ExampleWorkerInteractive : public MyWorker
{
public:
    ExampleWorkerInteractive() {}
    virtual ~ExampleWorkerInteractive() {}

    /// override MyWorker virtual method
    void Run() override {
        auto send_msg = std::make_shared<MyMsg>("this is ExampleWorkerInteractive req");
        SendMsg("actor.example_actor_interactive.#1", send_msg);
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
    return std::make_shared<ExampleActorInteractive>();
}

/* 创建worker实例函数 */
extern "C" std::shared_ptr<MyWorker> my_worker_create(const std::string& worker_name) {
    return std::make_shared<ExampleWorkerInteractive>();
}
