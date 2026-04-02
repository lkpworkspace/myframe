/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include <chrono>
#include <random>
#include <thread>
#include <unordered_map>

#include "myframe/log.h"
#include "myframe/msg.h"
#include "myframe/actor.h"

class ExampleActorPub : public myframe::Actor {
 public:
  int Init() override {
    Timeout("1000ms", 100);
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    if (msg->GetType() == MYFRAME_MSG_TYPE_TIMER) {
      auto pub_msg = std::make_shared<myframe::Msg>();
      pub_msg->SetData("hello, this is pub msg");
      Publish(std::move(pub_msg));
      Timeout("1000ms", 100);
    }
  }
};

class ExampleActorSub : public myframe::Actor {
 public:
  int Init() override {
    Subscribe("actor.ExampleActorPub.1");
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    LOG(INFO) << "-----> get pub msg: " << *msg;
  }
};

extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "ExampleActorPub") {
    return std::make_shared<ExampleActorPub>();
  }
  if (actor_name == "ExampleActorSub") {
    return std::make_shared<ExampleActorSub>();
  }
  return nullptr;
}
