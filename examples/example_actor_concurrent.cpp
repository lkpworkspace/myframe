/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include <chrono>
#include <random>
#include <thread>
#include <unordered_map>

#include <glog/logging.h>

#include "myframe/msg.h"
#include "myframe/actor.h"

class ExampleActorConcurrent : public myframe::Actor {
 public:
  int random(int min, int max) {
    std::random_device seed;
    std::ranlux48 engine(seed());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(engine);
  }

  int Init(const char* param) override {
    (void)param;
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    if (msg->GetSrc() == "actor.example_actor_concurrent_trigger.#1") {
      int cost_ms = random(100, 500);
      LOG(INFO) << "-----> " << GetActorName() << " begin runing...";
      std::this_thread::sleep_for(std::chrono::milliseconds(cost_ms));
      LOG(INFO) << "-----> " << GetActorName() << " process end, cost "
                << cost_ms << " ms";
      auto mailbox = GetMailbox();
      mailbox->Send(msg->GetSrc(), std::make_shared<myframe::Msg>(""));
    }
  }
};

class ExampleActorConcurrentTrigger : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
    state_ = {
        {"actor.example_actor_concurrent.#1", false},
        {"actor.example_actor_concurrent.#2", false},
        {"actor.example_actor_concurrent.#3", false},
    };
    LOG(INFO) << "begin concurrent task...";
    for (auto it : state_) {
      auto mailbox = GetMailbox();
      mailbox->Send(it.first, std::make_shared<myframe::Msg>(""));
    }
    return 0;
  }

  bool WaitEnd(const std::string& name) {
    state_[name] = true;
    for (auto it : state_) {
      if (it.second == false) {
        return false;
      }
    }
    return true;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    if (WaitEnd(msg->GetSrc())) {
      LOG(INFO) << "concurrent task finished";
    }
  }

 private:
  std::unordered_map<std::string, bool> state_;
};

extern "C" std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "example_actor_concurrent") {
    return std::make_shared<ExampleActorConcurrent>();
  }
  if (actor_name == "example_actor_concurrent_trigger") {
    return std::make_shared<ExampleActorConcurrentTrigger>();
  }
  return nullptr;
}
