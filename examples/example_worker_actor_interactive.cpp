/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include <chrono>
#include <thread>

#include <glog/logging.h>

#include "myframe/actor.h"
#include "myframe/msg.h"
#include "myframe/worker.h"

/// 回显worker发来的消息
class ExampleActorInteractive : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    auto mailbox = GetMailbox();
    mailbox->Send(msg->GetSrc(),
      std::make_shared<myframe::Msg>("this is ExampleActorInteractive resp"));
  }
};

/// 给ExampleActorInteractive发送消息，并打印收到的消息
class ExampleWorkerInteractive : public myframe::Worker {
 public:
  ExampleWorkerInteractive() {}
  virtual ~ExampleWorkerInteractive() {}

  /// override Worker virtual method
  void Run() override {
    auto mailbox = GetMailbox();
    auto send_msg =
        std::make_shared<myframe::Msg>("this is ExampleWorkerInteractive req");
    mailbox->Send("actor.example_actor_interactive.#1", send_msg);
    if (-1 == DispatchAndWaitMsg()) {
      return;
    }
    while (1) {
      const auto& msg = mailbox->PopRecv();
      if (msg == nullptr) {
        break;
      }
      LOG(INFO) << *msg << ": " << msg->GetData();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
};

/* 创建actor实例函数 */
extern "C" std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "example_actor_interactive") {
    return std::make_shared<ExampleActorInteractive>();
  }
  return nullptr;
}

/* 创建worker实例函数 */
extern "C" std::shared_ptr<myframe::Worker> worker_create(
    const std::string& worker_name) {
  if (worker_name == "example_worker_interactive") {
    return std::make_shared<ExampleWorkerInteractive>();
  }
  return nullptr;
}
