/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
/*
示例概述：
  示范应用框架的定时器

创建对象：
  actor.ExampleActorTimer.1

执行逻辑：
  actor.ExampleActorTimer.1每隔1秒触发一次打印超时消息
*/
#include "myframe/log.h"
#include "myframe/msg.h"
#include "myframe/actor.h"

class ExampleActorTimer : public myframe::Actor {
 public:
  int Init() override {
    /* 设置超时时间为 100 * 10 ms */
    Timeout("1000ms", 10);
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    if (msg->GetType() == MYFRAME_MSG_TYPE_TIMER &&
        msg->GetName() == "1000ms") {
      /* 设置下一次超时时间 100 * 10 ms */
      Timeout("1000ms", 100);
      LOG(INFO) << *msg << ": " << "timeout";
    }
  }
};

extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "ExampleActorTimer") {
    return std::make_shared<ExampleActorTimer>();
  }
  return nullptr;
}
