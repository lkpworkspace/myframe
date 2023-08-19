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
#include <glog/logging.h>

#include "myframe/common.h"
#include "myframe/log.h"
#include "myframe/msg.h"
#include "myframe/actor.h"
#include "myframe/mod_manager.h"
#include "myframe/app.h"

#include "performance_test_config.h"

class EchoActorTest : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
    LOG(INFO) << "init EchoActorTest";
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    LOG(INFO) << "recv " << msg->GetSrc() << ":" << msg->GetData();
    if (msg->GetData() == "hello") {
      auto re = std::make_shared<myframe::Msg>(
        "resp:" + std::to_string(seq_++));
      auto mailbox = GetMailbox();
      mailbox->Send(msg->GetSrc(), re);
    }
  }

 private:
  int seq_{0};
};

int main() {
  auto lib_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LIB_DIR);
  auto log_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LOG_DIR);

  myframe::InitLog(log_dir, "app_send_test");

  auto app = std::make_shared<myframe::App>();
  if (false == app->Init(lib_dir, 4)) {
    LOG(ERROR) << "Init failed";
    return -1;
  }

  // mod manager
  auto& mod = app->GetModManager();

  // 注册echo Actor
  mod->RegActor("EchoActorTest", [](const std::string&) {
      return std::make_shared<EchoActorTest>();
  });
  auto actor = mod->CreateActorInst("class", "EchoActorTest");
  app->AddActor("1", "", actor);

  // 创建一个线程请求服务
  std::thread th([&]() {
    int cnt = 100;
    while (cnt--) {
      auto resp = app->SendRequest(
        "actor.EchoActorTest.1",
        std::make_shared<myframe::Msg>("hello"));
      LOG(INFO) << "get resp: " << resp->GetData();
      app->Send(
        "actor.EchoActorTest.1",
        std::make_shared<myframe::Msg>("world"));
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    app->Quit();
  });

  app->Exec();
  th.join();
  return 0;
}
