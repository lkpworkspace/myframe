/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include <vector>
#include <unordered_map>

#include "myframe/log.h"
#include "myframe/msg.h"
#include "myframe/actor.h"

/**
模拟进程间/机器间通信的节点组件
*/
class ExampleNode : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    LOG(INFO) << "ExampleNode recv msg:" << *msg;
    // 接收到订阅消息,模拟创建订阅topic并加入到订阅列表
    if (msg->GetType() == "SUBSCRIBE"
        && sub_list_.find(msg->GetSrc()) == sub_list_.end()) {
      // 订阅 msg->GetDst() 对应的topic
      // ...
      // 加入到订阅列表
      sub_list_[msg->GetDst()].push_back(msg->GetSrc());
      return;
    }
    // 接收到发布消息
    // 如果没有发布topic，则创建发布topic
    // 发送消息给发布topic
    // ...

    // 注意：这里示例代码直接发送给订阅组件，实际不应该这么做
    if (sub_list_.find(msg->GetDst()) != sub_list_.end()) {
      const auto& sub_list = sub_list_[msg->GetDst()];
      auto mailbox = GetMailbox();
      for (const auto& sub : sub_list) {
        auto send_msg = std::make_shared<myframe::Msg>();
        send_msg->SetSrc(msg->GetDst());
        send_msg->SetDst(sub);
        mailbox->Send(send_msg);
      }
    }
  }

 private:
  // key: sub topic name
  // value: sub com list
  std::unordered_map<std::string, std::vector<std::string>> sub_list_;

  // key: pub topic name
  // value: pub topic object
  // std::unordered_map<std::string, pub_obj> pub_list_;
};

/**
模拟发送消息给另外一个进程组件(actor.other.process)
*/
class ExampleNodePub : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
    Timeout("1000ms", 100);
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    if (msg->GetType() == "TIMER") {
      auto mailbox = GetMailbox();
      LOG(INFO) << "ExampleNodePub send msg to actor.other.process";
      mailbox->Send("actor.other.process", "");
      Timeout("1000ms", 100);
    }
  }
};

/**
模拟订阅另外一个进程消息的组件
  1. 订阅另外一个进程组件(actor.other.process)的消息
  2. 接收消息并打印
*/
class ExampleNodeSub : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
    Subscribe("actor.other.process");
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    LOG(INFO) << "ExampleNodeSub recv msg " << *msg;
  }
};

extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "node") {
    return std::make_shared<ExampleNode>();
  }
  if (actor_name == "node_pub") {
    return std::make_shared<ExampleNodePub>();
  }
  if (actor_name == "node_sub") {
    return std::make_shared<ExampleNodeSub>();
  }
  return nullptr;
}
