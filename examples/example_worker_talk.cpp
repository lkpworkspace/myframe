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

class ExampleWorkerTalk : public myframe::Worker {
 public:
  ExampleWorkerTalk() {}
  virtual ~ExampleWorkerTalk() {}

  void Run() override {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // recv msg from some device
    // ...
    // send msg to actor/worker
    // eg: Send("actor.xx.xx", std::make_shared<Msg>("hello,world"));
    LOG(INFO) << "talk worker do something";
    DispatchMsg();
  }
};

/* 创建worker实例函数 */
extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Worker> worker_create(
    const std::string& worker_name) {
  if (worker_name == "example_worker_talk") {
    return std::make_shared<ExampleWorkerTalk>();
  }
  return nullptr;
}
