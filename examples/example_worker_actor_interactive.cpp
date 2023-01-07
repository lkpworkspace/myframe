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
  int Init(const char* param) override { return 0; }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    Send(msg->GetSrc(), std::make_shared<myframe::Msg>(
                            "this is ExampleActorInteractive resp"));
  }
};

/// 给ExampleActorInteractive发送消息，并打印收到的消息
class ExampleWorkerInteractive : public myframe::Worker {
 public:
  ExampleWorkerInteractive() {}
  virtual ~ExampleWorkerInteractive() {}

  /// override Worker virtual method
  void Run() override {
    auto send_msg =
        std::make_shared<myframe::Msg>("this is ExampleWorkerInteractive req");
    SendMsg("actor.example_actor_interactive.#1", send_msg);
    DispatchAndWaitMsg();
    while (1) {
      const auto& msg = GetRecvMsg();
      if (msg == nullptr) {
        break;
      }
      LOG(INFO) << *msg << ": " << msg->GetData();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
};

/* 创建actor实例函数 */
extern "C" std::shared_ptr<myframe::Actor> my_actor_create(
    const std::string& actor_name) {
  return std::make_shared<ExampleActorInteractive>();
}

/* 创建worker实例函数 */
extern "C" std::shared_ptr<myframe::Worker> my_worker_create(
    const std::string& worker_name) {
  return std::make_shared<ExampleWorkerInteractive>();
}
