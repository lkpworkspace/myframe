/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
/*
示例概述：
  示范Worker通过网络输入输出消息

创建对象：
  worker.ExampleWorkerInput.1
  worker.ExampleWorkerOutput.1

执行逻辑：
  worker.ExampleWorkerInput.1接收网络消息并将消息转发给框架；
  worker.ExampleWorkerOutput.1接收框架消息后通过网络发送出去。
*/
#include <chrono>
#include <thread>

#include "myframe/log.h"
#include "myframe/actor.h"
#include "myframe/msg.h"
#include "myframe/worker.h"

class ExampleWorkerInput : public myframe::Worker {
 public:
  ExampleWorkerInput() {}
  virtual ~ExampleWorkerInput() {}

  void Run() override {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // recv msg from some device
    // ...
    // send msg to actor/worker
    // eg: Send("actor.xx.xx", std::make_shared<Msg>("hello,world"));
    LOG(INFO) << "Input worker do something";
    DispatchMsg();
  }
};

class ExampleWorkerOutput : public myframe::Worker {
 public:
  ExampleWorkerOutput() {}
  virtual ~ExampleWorkerOutput() {}

  void Run() override {
    if (-1 == DispatchAndWaitMsg()) {
      return;
    }
    auto mailbox = GetMailbox();
    while (!mailbox->RunEmpty()) {
      const auto msg = mailbox->PopRun();
      // send msg by udp/tcp/zmq/...
      LOG(INFO) << "Output msg " << msg->GetData() << " ...";
    }
  }
};

/* 创建worker实例函数 */
extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Worker> worker_create(
    const std::string& worker_name) {
  if (worker_name == "ExampleWorkerOutput") {
    return std::make_shared<ExampleWorkerOutput>();
  }
  if (worker_name == "ExampleWorkerInput") {
    return std::make_shared<ExampleWorkerInput>();
  }
  return nullptr;
}
