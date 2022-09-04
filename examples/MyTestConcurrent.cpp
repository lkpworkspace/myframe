#include <random>
#include <thread>
#include <chrono>
#include <unordered_map>

#include <glog/logging.h>

#include "myframe/MyActor.h"
#include "myframe/MyMsg.h"

class MyTestConcurrent : public MyActor
{
public:
    int random(int min, int max) {
        std::random_device seed;
	    std::ranlux48 engine(seed());
        std::uniform_int_distribution<> distrib(min, max);
        return distrib(engine);
    }

    int Init(const char* param) override {
        return 0;
    }

    void CB(const std::shared_ptr<const MyMsg>& msg) override {
        if (msg->GetSrc() == "actor.test_concurrent_trigger.#1") {
            int cost_ms = random(100, 500);
            LOG(INFO) << "-----> " << GetActorName() << " begin runing...";
            std::this_thread::sleep_for(std::chrono::milliseconds(cost_ms));
            LOG(INFO) << "-----> " << GetActorName() << " process end, cost " << cost_ms << " ms";
            Send(msg->GetSrc(), std::make_shared<MyMsg>(""));
        }
    }
};

class MyTestConcurrentTrigger : public MyActor
{
public:
    int Init(const char* param) override {
        _state = {
            {"actor.test_concurrent.#1", false},
            {"actor.test_concurrent.#2", false},
            {"actor.test_concurrent.#3", false},
        };
        LOG(INFO) << "begin concurrent task...";
        for (auto it : _state) {
            Send(it.first, std::make_shared<MyMsg>(""));
        }
        return 0;
    }

    bool WaitEnd(const std::string& name) {
        _state[name] = true;
        for (auto it : _state) {
            if (it.second == false) {
                return false;
            }
        }
        return true;
    }

    void CB(const std::shared_ptr<const MyMsg>& msg) override {
        if (WaitEnd(msg->GetSrc())) {
            LOG(INFO) << "concurrent task finished";
        }
    }

private:
    std::unordered_map<std::string, bool> _state;
};

extern "C" std::shared_ptr<MyActor> my_actor_create(const std::string& actor_name) {
    if (actor_name == "test_concurrent") {
        return std::make_shared<MyTestConcurrent>();
    }
    if (actor_name == "test_concurrent_trigger") {
        return std::make_shared<MyTestConcurrentTrigger>();
    }
    return nullptr;
}
