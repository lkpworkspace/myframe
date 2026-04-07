/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
/*
示例概述：
  示范如何在Worker/Actor间传递消息

创建对象：
  actor.ExampleActorTransMsg.1
  worker.ExampleWorkerTransMsg.1

执行逻辑：
  worker.ExampleWorkerTransMsg.1发送消息给actor.ExampleActorTransMsg.1并等待回复，
  actor.ExampleActorTransMsg.1收到消息会回复消息给worker.ExampleWorkerTransMsg.1。
*/
#include <chrono>
#include <thread>

#include "myframe/log.h"
#include "myframe/actor.h"
#include "myframe/msg.h"
#include "myframe/worker.h"

/// 回显worker发来的消息
class ExampleActorTransMsg : public myframe::Actor {
 public:
  int Init() override {
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    auto mailbox = GetMailbox();
    mailbox->Send(msg->GetSrc(),
      std::make_shared<myframe::Msg>("this is ExampleActorTransMsg resp"));
  }
};

/// 给ExampleActorTransMsg发送消息，并打印收到的消息
class ExampleWorkerTransMsg : public myframe::Worker {
 public:
  ExampleWorkerTransMsg() {}
  virtual ~ExampleWorkerTransMsg() {}

  void Run() override {
    auto mailbox = GetMailbox();
    auto send_msg =
        std::make_shared<myframe::Msg>("this is ExampleWorkerTransMsg req");
    mailbox->Send("actor.ExampleActorTransMsg.1", send_msg);
    if (-1 == DispatchAndWaitMsg()) {
      return;
    }
    while (1) {
      const auto msg = mailbox->PopRun();
      if (msg == nullptr) {
        break;
      }
      LOG(INFO) << *msg << ", data: " << msg->GetData();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
};

/* 创建actor实例函数 */
extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "ExampleActorTransMsg") {
    return std::make_shared<ExampleActorTransMsg>();
  }
  return nullptr;
}

/* 创建worker实例函数 */
extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Worker> worker_create(
    const std::string& worker_name) {
  if (worker_name == "ExampleWorkerTransMsg") {
    return std::make_shared<ExampleWorkerTransMsg>();
  }
  return nullptr;
}
