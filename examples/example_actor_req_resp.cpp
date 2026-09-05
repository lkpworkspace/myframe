/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
/*
示例概述：
  示范应用框架的请求回复功能

创建对象：
  actor.ExampleActorReq.1
  actor.ExampleActorResp.1
执行逻辑：
  actor.ExampleActorReq.1会定时发送请求消息
  actor.ExampleActorResp.1回复actor.ExampleActorReq.1的消息
  actor.ExampleActorReq.1会打印收到的响应消息。
*/
#include <chrono>
#include <random>
#include <thread>
#include <unordered_map>

#include "myframe/log.h"
#include "myframe/msg.h"
#include "myframe/actor.h"

class ExampleActorResp : public myframe::Actor {
 public:
  int Init() override {
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    if (msg->GetType() == MYFRAME_MSG_TYPE_REQUEST) {
      auto resp_msg = std::make_shared<myframe::Msg>();
      resp_msg->SetData(msg->GetData());
      Response(msg, std::move(resp_msg));
    }
  }
};

class ExampleActorReq : public myframe::Actor {
  int seq_num_{0};

 public:
  int Init() override {
    Timeout("1000ms", 100);
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    if (msg->GetType() == MYFRAME_MSG_TYPE_TIMER) {
      auto req_msg = std::make_shared<myframe::Msg>();
      std::string data = "hello, this is req msg ";
      data += std::to_string(seq_num_++);
      req_msg->SetData(data);
      req_msg->SetDst("actor.ExampleActorResp.1");
      Request(std::move(req_msg), [this](const std::shared_ptr<const myframe::Msg> resp_msg) {
        LOG(INFO) << "-----> get resp msg: " << *resp_msg << ", data: " << resp_msg->GetData();
      });
      Timeout("1000ms", 100);
    }
  }
};

extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "ExampleActorReq") {
    return std::make_shared<ExampleActorReq>();
  }
  if (actor_name == "ExampleActorResp") {
    return std::make_shared<ExampleActorResp>();
  }
  return nullptr;
}
