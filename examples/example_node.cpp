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

const char g_topic[] = "actor.ExampleNodePub.1";

/**
模拟进程间/机器间通信的节点组件
*/
class ExampleNode : public myframe::Actor {
 public:
  virtual ~ExampleNode() {
    // 发布topic发现退出消息
    // ...
  }

  int Init(const char* param) override {
    (void)param;
    // 启动订阅监听程序
    //   收到消息调用OnSubMsg()回调函数将消息发送给framework的组件

    // 启动topic发现程序
    //   监听topic发现消息并转发给framework
    //   监听topic退出消息并转发给framework
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    LOG(INFO) << "ExampleNode recv msg:" << *msg;

    // 接收到订阅消息,模拟创建订阅topic并加入到订阅列表
    if (msg->GetType() == "SUBSCRIBE"
        && sub_list_.find(msg->GetSrc()) == sub_list_.end()) {
      // 订阅 msg->GetDst() 对应的topic
      // ...
      // 发布topic发现消息
      // ...
      // 加入到订阅列表
      sub_list_[msg->GetDst()].push_back(msg->GetSrc());

      // 模拟另外一个进程收到订阅消息并转发给组件
      // 将订阅消息发送给要订阅的组件
      auto send_msg = std::make_shared<myframe::Msg>();
      send_msg->SetSrc(msg->GetSrc());
      send_msg->SetDst(msg->GetDst());
      send_msg->SetType("SUBSCRIBE");
      send_msg->SetDesc(msg->GetDesc());
      send_msg->SetTransMode(myframe::Msg::TransMode::kIntra);
      auto mailbox = GetMailbox();
      mailbox->Send(std::move(send_msg));
      return;
    }

    // 接收到发布消息
    // 搜索订阅发现列表是否存在(这个任务也可以交给framework去做)
    //  不存在就忽略(避免无人订阅还发送消息浪费资源)
    //  存在就创建topic并发送消息
    // ...

    // 模拟另外一个进程收到发布消息并转发给对应组件
    {
      auto mailbox = GetMailbox();
      auto send_msg = std::make_shared<myframe::Msg>();
      send_msg->operator=(*msg);
      send_msg->SetTransMode(myframe::Msg::TransMode::kIntra);
      mailbox->Send(std::move(send_msg));
    }
  }

 private:
  // 订阅回调函数
  void OnSubMsg(const std::shared_ptr<const myframe::Msg>& msg) {
    if (sub_list_.find(msg->GetDst()) != sub_list_.end()) {
      const auto& sub_list = sub_list_[msg->GetDst()];
      auto mailbox = GetMailbox();
      for (const auto& sub : sub_list) {
        auto send_msg = std::make_shared<myframe::Msg>();
        send_msg->SetSrc(msg->GetDst());
        send_msg->SetDst(sub);
        send_msg->SetTransMode(myframe::Msg::TransMode::kIntra);
        mailbox->Send(std::move(send_msg));
      }
    }
  }

  // key: sub topic name
  // value: sub com list
  std::unordered_map<std::string, std::vector<std::string>> sub_list_;

  // key: pub topic name
  // value: pub topic object
  // std::unordered_map<std::string, pub_obj> pub_list_;
};

/**
模拟发送消息给另外一个进程组件(g_topic)
*/
class ExampleNodePub : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
    Timeout("1000ms", 100);
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    LOG(INFO) << "ExampleNodePub recv msg:" << *msg;
    if (msg->GetType() == "SUBSCRIBE") {
      pub_list_.push_back(msg->GetSrc());
      return;
    }
    if (msg->GetType() == "TIMER") {
      auto mailbox = GetMailbox();
      for (const auto& pub : pub_list_) {
        LOG(INFO) << "ExampleNodePub send msg to " << pub;
        auto send_msg = std::make_shared<myframe::Msg>();
        send_msg->SetTransMode(myframe::Msg::TransMode::kDDS);
        mailbox->Send(pub, std::move(send_msg));
      }
      Timeout("1000ms", 100);
    }
  }

 private:
  std::vector<std::string> pub_list_;
};

/**
模拟订阅另外一个进程消息的组件
  1. 订阅另外一个进程组件(g_topic)的消息
  2. 接收消息并打印
*/
class ExampleNodeSub : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
    Subscribe(g_topic, "TEXT", myframe::Msg::TransMode::kDDS);
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
  if (actor_name == "ExampleNodePub") {
    return std::make_shared<ExampleNodePub>();
  }
  if (actor_name == "ExampleNodeSub") {
    return std::make_shared<ExampleNodeSub>();
  }
  return nullptr;
}
