/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include <chrono>
#include <random>
#include <thread>
#include <unordered_map>

#include <glog/logging.h>

#include "myframe/msg.h"
#include "myframe/actor.h"

class ExampleActorPub : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
    Timeout("1000ms", 100);
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    if (msg->GetType() == "SUBSCRIBE") {
      pub_list_.push_back(msg->GetSrc());
      return;
    }
    if (msg->GetType() == "TIMER") {
      auto mailbox = GetMailbox();
      for (size_t i = 0; i < pub_list_.size(); ++i) {
        mailbox->Send(pub_list_[i],
          std::make_shared<myframe::Msg>("pub msg"));
      }
      Timeout("1000ms", 100);
    }
  }
 private:
  std::vector<std::string> pub_list_;
};

class ExampleActorSub : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
    Subscribe("actor.example_b_pub.#1");
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    LOG(INFO) << "-----> get pub msg: " << *msg;
  }
};

extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "example_b_pub") {
    return std::make_shared<ExampleActorPub>();
  }
  if (actor_name == "example_a_sub") {
    return std::make_shared<ExampleActorSub>();
  }
  return nullptr;
}
