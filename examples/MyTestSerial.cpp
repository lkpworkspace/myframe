#include <random>
#include <thread>
#include <chrono>
#include <unordered_map>

#include <glog/logging.h>

#include "myframe/MyActor.h"
#include "myframe/MyMsg.h"

int random(int min, int max) {
    std::random_device seed;
    std::ranlux48 engine(seed());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(engine);
}

class MyTestSerial1 : public MyActor
{
public:
    int Init(const char* param) override {
        Send("actor.test_serial_1.#1", std::make_shared<MyMsg>(""));
        return 0;
    }

    void CB(const std::shared_ptr<const MyMsg>& msg) override {
        int cost_ms = random(100, 500);
        LOG(INFO) << "-----> begin runing task " << GetActorName() << "...";
        std::this_thread::sleep_for(std::chrono::milliseconds(cost_ms));
        LOG(INFO) << "-----> " << GetActorName() << " process end, cost " << cost_ms << " ms";
        Send("actor.test_serial_2.#1", std::make_shared<MyMsg>(""));
    }
};

class MyTestSerial2 : public MyActor
{
public:
    int Init(const char* param) override {
        return 0;
    }

    void CB(const std::shared_ptr<const MyMsg>& msg) override {
        int cost_ms = random(100, 500);
        LOG(INFO) << "-----> begin runing task " << GetActorName() << "...";
        std::this_thread::sleep_for(std::chrono::milliseconds(cost_ms));
        LOG(INFO) << "-----> " << GetActorName() << " process end, cost " << cost_ms << " ms";
        Send("actor.test_serial_3.#1", std::make_shared<MyMsg>(""));
    }
};

class MyTestSerial3 : public MyActor
{
public:
    int Init(const char* param) override {
        return 0;
    }

    void CB(const std::shared_ptr<const MyMsg>& msg) override {
        int cost_ms = random(100, 500);
        LOG(INFO) << "-----> begin runing task " << GetActorName() << "...";
        std::this_thread::sleep_for(std::chrono::milliseconds(cost_ms));
        LOG(INFO) << "-----> " << GetActorName() << " process end, cost " << cost_ms << " ms";        
    }
};

extern "C" std::shared_ptr<MyActor> my_actor_create(const std::string& actor_name) {
    if (actor_name == "test_serial_1") {
        return std::make_shared<MyTestSerial1>();
    }
    if (actor_name == "test_serial_2") {
        return std::make_shared<MyTestSerial2>();
    }
    if (actor_name == "test_serial_3") {
        return std::make_shared<MyTestSerial3>();
    }
    return nullptr;
}
