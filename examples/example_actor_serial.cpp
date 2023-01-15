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

int random(int min, int max) {
  std::random_device seed;
  std::ranlux48 engine(seed());
  std::uniform_int_distribution<> distrib(min, max);
  return distrib(engine);
}

class ExampleActorSerial1 : public myframe::Actor {
 public:
  int Init(const char* param) override {
    auto mailbox = GetMailbox();
    mailbox->Send("actor.example_serial1.#1",
      std::make_shared<myframe::Msg>(""));
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    int cost_ms = random(100, 500);
    LOG(INFO) << "-----> begin runing task " << GetActorName() << "...";
    std::this_thread::sleep_for(std::chrono::milliseconds(cost_ms));
    LOG(INFO) << "-----> " << GetActorName() << " process end, cost " << cost_ms
              << " ms";
    auto mailbox = GetMailbox();
    mailbox->Send("actor.example_serial2.#1",
      std::make_shared<myframe::Msg>(""));
  }
};

class ExampleActorSerial2 : public myframe::Actor {
 public:
  int Init(const char* param) override { return 0; }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    int cost_ms = random(100, 500);
    LOG(INFO) << "-----> begin runing task " << GetActorName() << "...";
    std::this_thread::sleep_for(std::chrono::milliseconds(cost_ms));
    LOG(INFO) << "-----> " << GetActorName() << " process end, cost " << cost_ms
              << " ms";
    auto mailbox = GetMailbox();
    mailbox->Send("actor.example_serial3.#1",
      std::make_shared<myframe::Msg>(""));
  }
};

class ExampleActorSerial3 : public myframe::Actor {
 public:
  int Init(const char* param) override { return 0; }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    int cost_ms = random(100, 500);
    LOG(INFO) << "-----> begin runing task " << GetActorName() << "...";
    std::this_thread::sleep_for(std::chrono::milliseconds(cost_ms));
    LOG(INFO) << "-----> " << GetActorName() << " process end, cost " << cost_ms
              << " ms";
  }
};

extern "C" std::shared_ptr<myframe::Actor> my_actor_create(
    const std::string& actor_name) {
  if (actor_name == "example_serial1") {
    return std::make_shared<ExampleActorSerial1>();
  }
  if (actor_name == "example_serial2") {
    return std::make_shared<ExampleActorSerial2>();
  }
  if (actor_name == "example_serial3") {
    return std::make_shared<ExampleActorSerial3>();
  }
  return nullptr;
}
