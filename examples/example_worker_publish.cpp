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

class ExampleWorkerPublic : public myframe::Worker {
 public:
  ExampleWorkerPublic() {}
  virtual ~ExampleWorkerPublic() {}

  void Run() override {
    DispatchAndWaitMsg();
    auto mailbox = GetMailbox();
    while (!mailbox->RecvEmpty()) {
      const auto& msg = mailbox->PopRecv();
      // send msg by udp/tcp/zmq/...
      LOG(INFO) << "public msg " << msg->GetData() << " ...";
    }
  }
};

/* 创建worker实例函数 */
extern "C" std::shared_ptr<myframe::Worker> worker_create(
    const std::string& worker_name) {
  if (worker_name == "example_worker_publish") {
    return std::make_shared<ExampleWorkerPublic>();
  }
  return nullptr;
}
