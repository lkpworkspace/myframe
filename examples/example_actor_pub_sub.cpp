/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
/*
示例概述：
  示范应用框架的发布订阅功能

创建对象：
  actor.ExampleActorPub.1
  actor.ExampleActorSub.1

执行逻辑：
  actor.ExampleActorPub.1会定时发布消息
  actor.ExampleActorSub.1初始化会订阅actor.ExampleActorPub.1的消息
  actor.ExampleActorSub.1会打印收到的订阅消息。
*/
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
      std::string data = "hello, this is pub msg ";
      data += std::to_string(seq_num_++);
      pub_msg->SetData(data);
      Publish(std::move(pub_msg));
      Timeout("1000ms", 100);
    }
  }

 private:
  int seq_num_{0};
};

class ExampleActorSub : public myframe::Actor {
 public:
  int Init() override {
    Subscribe("actor.ExampleActorPub.1");
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    LOG(INFO) << "-----> get pub msg: " << *msg << ", data: " << msg->GetData();
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
