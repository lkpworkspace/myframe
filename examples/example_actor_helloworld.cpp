/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#include <glog/logging.h>

#include "myframe/msg.h"
#include "myframe/actor.h"

/*
    自己给自己发送一条消息
*/
class ExampleActorHelloWorld : public myframe::Actor {
 public:
  /* actor模块加载完毕后调用 */
  int Init(const char* param) override {
    (void)param;
    auto mailbox = GetMailbox();
    /* 构造 hello,world 消息发送给自己 */
    mailbox->Send(mailbox->Addr(), std::string("hello,world"));
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    /* 获得文本消息， 打印 源actor地址 目的actor地址 消息内容*/
    auto data = msg->GetAnyData<std::string>();
    LOG(INFO) << *msg << ": " << data;
  }
};

/* 创建actor模块实例函数 */
extern "C" std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "example") {
    return std::make_shared<ExampleActorHelloWorld>();
  }
  return nullptr;
}
