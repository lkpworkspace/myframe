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
extern "C" std::shared_ptr<myframe::Worker> my_worker_create(
    const std::string& worker_name) {
  if (worker_name == "worker.example_worker_talk.#1") {
    return std::make_shared<ExampleWorkerTalk>();
  }
  return nullptr;
}
