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
        return 0;
    }

    void CB(const std::shared_ptr<const MyMsg>& msg) override {
        if (msg->GetMsgType() == "TEXT") {
            Send(msg->GetSrc(), std::make_shared<MyMsg>("this is MyTestWorkerActor resp"));
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
        auto send_msg = std::make_shared<MyMsg>("this is MyTestWorker req");
        SendMsg("actor.testworker.#1", send_msg);
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
    return std::make_shared<MyTestWorkerActor>();
}

/* 创建worker实例函数 */
extern "C" std::shared_ptr<MyWorker> my_worker_create(const std::string& worker_name) {
    return std::make_shared<MyTestWorker>();
}
