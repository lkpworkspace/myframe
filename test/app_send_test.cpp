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

class EchoActorTest : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
    LOG(INFO) << "init EchoActorTest";
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    LOG(INFO) << "recv " << msg->GetSrc() << ":" << msg->GetData();
  }
};

int main() {
  auto lib_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LIB_DIR).string();
  auto log_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LOG_DIR).string();

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

  // 测试Send函数
  std::mutex mtx;
  int th_cnt = 5;
  int exit_th_cnt = 0;
  int send_cnt = 10000;
  std::vector<std::thread> th_vec;
  for (int i = 0; i < th_cnt; ++i) {
    th_vec.push_back(std::thread([&, i](){
      int cnt = send_cnt;
      while (cnt--) {
        auto msg = std::make_shared<myframe::Msg>(
          "world " + std::to_string(i));
        msg->SetDst("actor.EchoActorTest.1");
        app->Send(std::move(msg));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      std::lock_guard<std::mutex> g(mtx);
      LOG(INFO) << "user thread " << i << " exit";
      ++exit_th_cnt;
      if (exit_th_cnt == th_cnt) {
        app->Quit();
      }
    }));
  }

  app->Exec();
  for (int i = 0; i < th_cnt; ++i) {
    if (th_vec[i].joinable()) {
      th_vec[i].join();
    }
  }
  return 0;
}
