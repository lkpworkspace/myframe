/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
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

#include "performance_test_config.h"

/*
设置run_queue_size为2，
给自己一次性发送10条消息，
理论上每执行两条消息就应该让出线程资源，
下一次执行应该在另外一个线程，
可以通过打印线程号判断是否让出资源。
*/
class RunQueueTest : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
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
  auto lib_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LIB_DIR).string();
  auto log_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LOG_DIR).string();

  myframe::InitLog(log_dir, "actor_run_queue_test");

  auto app = std::make_shared<myframe::App>();
  if (false == app->Init(
    lib_dir,
    4,  // 线程池大小
    2,  // ConnectEvent池大小
    10, // 接收队列最大值警告
    -1, // 接收队列最大值（全局）
    2  // 运行队列最大值（全局）
  )) {
    LOG(ERROR) << "Init failed";
    return -1;
  }

  // mod manager
  auto& mod = app->GetModManager();

  // 注册echo Actor
  mod->RegActor("RunQueueTest", [](const std::string&) {
      return std::make_shared<RunQueueTest>();
  });
  auto actor = mod->CreateActorInst("class", "RunQueueTest");
  app->AddActor("1", "", actor);

  return app->Exec();
}
