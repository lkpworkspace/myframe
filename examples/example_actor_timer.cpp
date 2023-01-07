/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include <glog/logging.h>

#include "myframe/msg.h"
#include "myframe/actor.h"

class ExampleActorTimer : public myframe::Actor {
 public:
  int Init(const char* param) override {
    /* 设置超时时间为 100 * 10 ms */
    Timeout("1000ms", 10);
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    if (msg->GetType() == "TIMER" && msg->GetDesc() == "1000ms") {
      /* 设置下一次超时时间 100 * 10 ms */
      Timeout("1000ms", 100);
      LOG(INFO) << *msg << ": " << "timeout";
    }
  }
};

extern "C" std::shared_ptr<myframe::Actor> my_actor_create(
    const std::string& actor_name) {
  return std::make_shared<ExampleActorTimer>();
}
