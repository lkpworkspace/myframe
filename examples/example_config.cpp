/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include <chrono>
#include <thread>

#include <glog/logging.h>

#include "myframe/msg.h"
#include "myframe/actor.h"
#include "myframe/worker.h"

class ExampleActorConfig : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
    auto conf = GetConfig();
    LOG(INFO) << GetActorName() << " conf " << conf->toStyledString();
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    (void)msg;
  }
};

class ExampleWorkerConfig : public myframe::Worker {
 public:
  ExampleWorkerConfig() {}
  virtual ~ExampleWorkerConfig() {}

  void Init() override {
    auto conf = GetConfig();
    LOG(INFO) << GetWorkerName() << " conf " << conf->toStyledString();
  }
  void Run() override {
    Stop();
  }
};

/* 创建actor实例函数 */
extern "C" std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "example_actor_config") {
    return std::make_shared<ExampleActorConfig>();
  }
  return nullptr;
}

/* 创建worker实例函数 */
extern "C" std::shared_ptr<myframe::Worker> worker_create(
    const std::string& worker_name) {
  if (worker_name == "example_worker_config") {
    return std::make_shared<ExampleWorkerConfig>();
  }
  return nullptr;
}
