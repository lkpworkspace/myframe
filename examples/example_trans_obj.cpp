/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include <chrono>
#include <thread>

#include "myframe/log.h"
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
    (void)param;
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
extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "example_actor_trans_obj") {
    return std::make_shared<ExampleActorTransObj>();
  }
  return nullptr;
}

/* 创建worker实例函数 */
extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Worker> worker_create(
    const std::string& worker_name) {
  if (worker_name == "example_worker_trans_obj") {
    return std::make_shared<ExampleWorkerTransObj>();
  }
  return nullptr;
}
