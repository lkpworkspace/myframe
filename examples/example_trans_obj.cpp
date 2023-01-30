/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include <chrono>
#include <thread>

#include <glog/logging.h>

#include "myframe/msg.h"
#include "myframe/actor.h"
#include "myframe/worker.h"

class TransObj {
 public:
  TransObj(const std::string& a, const std::string& b)
    : a_(a)
    , b_(b)
  {}

  int Sum() {
    return std::stoi(a_) + std::stoi(b_);
  }

 private:
  std::string a_;
  std::string b_;
};

class ExampleActorTransObj : public myframe::Actor {
 public:
  int Init(const char* param) override {
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    auto obj = msg->GetAnyData<std::shared_ptr<TransObj>>();
    LOG(INFO) << *msg << " res " << obj->Sum();
  }
};

class ExampleWorkerTransObj : public myframe::Worker {
 public:
  void Run() override {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    auto obj = std::make_shared<TransObj>(
      std::to_string(a_),
      std::to_string(b_));
    auto mailbox = GetMailbox();
    mailbox->Send("actor.example_actor_trans_obj.#1", obj);
    DispatchMsg();
    a_++;
    b_++;
  }
 private:
  int a_{0};
  int b_{0};
};

/* 创建actor实例函数 */
extern "C" std::shared_ptr<myframe::Actor> my_actor_create(
    const std::string& actor_name) {
  return std::make_shared<ExampleActorTransObj>();
}

/* 创建worker实例函数 */
extern "C" std::shared_ptr<myframe::Worker> my_worker_create(
    const std::string& worker_name) {
  return std::make_shared<ExampleWorkerTransObj>();
}
