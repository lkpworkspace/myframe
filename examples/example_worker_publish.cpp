/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include <chrono>
#include <thread>

#include "myframe/log.h"
#include "myframe/actor.h"
#include "myframe/msg.h"
#include "myframe/worker.h"

class ExampleWorkerPublic : public myframe::Worker {
 public:
  ExampleWorkerPublic() {}
  virtual ~ExampleWorkerPublic() {}

  void Run() override {
    if (-1 == DispatchAndWaitMsg()) {
      return;
    }
    auto mailbox = GetMailbox();
    while (!mailbox->RunEmpty()) {
      const auto msg = mailbox->PopRun();
      // send msg by udp/tcp/zmq/...
      LOG(INFO) << "public msg " << msg->GetData() << " ...";
    }
  }
};

/* 创建worker实例函数 */
extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Worker> worker_create(
    const std::string& worker_name) {
  if (worker_name == "example_worker_publish") {
    return std::make_shared<ExampleWorkerPublic>();
  }
  return nullptr;
}
