#include <thread>
#include <chrono>

#include <glog/logging.h>

#include "myframe/MyMsg.h"
#include "myframe/MyActor.h"
#include "myframe/MyWorker.h"

using namespace myframe;

class ExampleWorkerTalk : public MyWorker
{
public:
    ExampleWorkerTalk() {}
    virtual ~ExampleWorkerTalk() {}

    void Run() override {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // TODO: send msg to actor/worker
        // eg: Send("actor.xx.xx", std::make_shared<MyMsg>("hello,world"));
        LOG(INFO) << "talk worker do something";
        DispatchMsg();
    }
};

/* 创建worker实例函数 */
extern "C" std::shared_ptr<MyWorker> my_worker_create(const std::string& worker_name) {
    return std::make_shared<ExampleWorkerTalk>();
}
