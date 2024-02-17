/****************************************************************************
Copyright (c) 2019, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include <memory>
#include <chrono>
#include <thread>
#include <iostream>

#include "export.h"
#include "myframe/msg.h"
#include "myframe/actor.h"
#include "myframe/worker.h"

class @template_name@Actor : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    /* print recv msg */
    std::cout << *msg << ": " << msg->GetData() << std::endl;
    /* resp msg */
    auto mailbox = GetMailbox();
    mailbox->Send(msg->GetSrc(),
      std::make_shared<myframe::Msg>("this is template actor resp"));
  }
};

class @template_name@Worker : public myframe::Worker {
 public:
  @template_name@Worker() {}
  virtual ~@template_name@Worker() {}

  void Run() override {
    /* send msg to actor.@template_name@ and recv resps msg */
    auto mailbox = GetMailbox();
    mailbox->Send("actor.@template_name@.@template_name@1",
            std::make_shared<myframe::Msg>("this is template worker req"));
    DispatchAndWaitMsg();
    while (1) {
      const auto msg = mailbox->PopRecv();
      if (msg == nullptr) {
        break;
      }
      std::cout << *msg << ": " << msg->GetData() << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
};

/* create actor instance */
extern "C" MYFRAME_SUBMODULE_EXPORT std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "@template_name@") {
    return std::make_shared<@template_name@Actor>();
  }
  return nullptr;
}

/* create worker instance */
extern "C" MYFRAME_SUBMODULE_EXPORT std::shared_ptr<myframe::Worker> worker_create(
    const std::string& worker_name) {
  if (worker_name == "@template_name@") {
    return std::make_shared<@template_name@Worker>();
  }
  return nullptr;
}
