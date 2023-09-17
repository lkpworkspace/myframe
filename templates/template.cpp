/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include <memory>
#include <chrono>
#include <thread>

#include <glog/logging.h>

#include "myframe/msg.h"
#include "myframe/actor.h"
#include "myframe/worker.h"

class @template_name@Actor : public myframe::Actor {
 public:
  /* actor模块加载完毕后调用 */
  int Init(const char* param) override {
    (void)param;
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    /* 获得文本消息， 打印 源地址 目的地址 消息内容*/
    LOG(INFO) << *msg;
    /* 回复消息 */
    auto mailbox = GetMailbox();
    mailbox->Send(msg->GetSrc(),
      std::make_shared<myframe::Msg>("this is actor resp"));
  }
};

class @template_name@Worker : public myframe::Worker {
 public:
  @template_name@Worker() {}
  virtual ~@template_name@Worker() {}

  /* 框架会循环调用该函数 */
  void Run() override {
    /* 给 actor.@template_name@ 发送消息，并接收回复消息 */
    auto mailbox = GetMailbox();
    mailbox->Send("actor.@template_name@.@template_name@1",
            std::make_shared<myframe::Msg>("this is template worker req"));
    DispatchAndWaitMsg();
    while (1) {
      const auto msg = mailbox->PopRecv();
      if (msg == nullptr) {
        break;
      }
      LOG(INFO) << *msg << ": " << msg->GetData();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
};

/* 创建actor实例函数 */
extern "C" std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "@template_name@") {
    return std::make_shared<@template_name@Actor>();
  }
  return nullptr;
}

/* 创建worker实例函数 */
extern "C" std::shared_ptr<myframe::Worker> worker_create(
    const std::string& worker_name) {
  if (worker_name == "@template_name@") {
    return std::make_shared<@template_name@Worker>();
  }
  return nullptr;
}
