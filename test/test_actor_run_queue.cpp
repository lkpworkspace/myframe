/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
/*
设置run_queue_size为2，
给自己一次性发送10条消息，
理论上每执行两条消息就应该让出线程资源，
下一次执行应该在另外一个线程，
可以通过打印线程号判断是否让出资源。
*/
#include <algorithm>
#include <numeric>
#include <vector>
#include <thread>
#include <chrono>
#include "myframe/common.h"
#include "myframe/log.h"
#include "myframe/msg.h"
#include "myframe/actor.h"
#include "myframe/mod_manager.h"
#include "myframe/app.h"

#include "test_config.h"

class RunQueueTest : public myframe::Actor {
 public:
  int Init() override {
    for (int i = 0; i < send_cnt_; ++i) {
        auto msg = std::make_shared<myframe::Msg>("hello");
        GetMailbox()->Send(GetActorName(), std::move(msg));
    }
    LOG(INFO) << "init RunQueueTest";
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    LOG(INFO) << "recv " << msg->GetSrc() << ":" << msg->GetData();
    recv_cnt_++;
    if (recv_cnt_ == send_cnt_) {
      GetApp()->Quit();
    }
  }

 private:
  int recv_cnt_{0};
  int send_cnt_{10};
};

int main() {
  auto log_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LOG_DIR).string();
  myframe::InitLog(log_dir, "test_actor_run_queue");

  auto app = std::make_shared<myframe::App>();
  myframe::Arguments args;
  if (false == app->Init(args)) {
    LOG(ERROR) << "Init failed";
    return -1;
  }

  // 注册/创建echo Actor
  auto& mod = app->GetModManager();
  mod->RegActor("RunQueueTest", [](const std::string&) {
      return std::make_shared<RunQueueTest>();
  });
  auto actor = mod->CreateActorInst("class", "RunQueueTest", "1");
  app->AddActor(actor);

  return app->Exec();
}
