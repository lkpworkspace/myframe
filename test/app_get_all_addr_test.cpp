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

class TmpTest : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
    LOG(INFO) << "init TmpTest";
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    (void)msg;
  }
};

int main() {
  auto lib_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LIB_DIR).string();
  auto log_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LOG_DIR).string();

  myframe::InitLog(log_dir, "app_get_all_addr_test");

  auto app = std::make_shared<myframe::App>();
  if (false == app->Init(lib_dir, 4)) {
    LOG(ERROR) << "Init failed";
    return -1;
  }

  // mod manager
  auto& mod = app->GetModManager();

  // 注册Tmp Actor
  mod->RegActor("TmpTest", [](const std::string&) {
      return std::make_shared<TmpTest>();
  });
  auto actor = mod->CreateActorInst("class", "TmpTest");
  app->AddActor("1", "", actor);
  auto actor2 = mod->CreateActorInst("class", "TmpTest");
  app->AddActor("2", "", actor2);

  // 请求获得所有用户模块地址
  std::mutex mtx;
  int th_cnt = 4;
  int exit_th_cnt = 0;
  int send_cnt = 1000;
  std::vector<std::thread> th_vec;
  for (int i = 0; i < th_cnt; ++i) {
    th_vec.push_back(std::thread([&, i](){
      while (app->GetState() != myframe::App::State::kRunning) {
        std::this_thread::sleep_for(
          std::chrono::milliseconds(100));
      }
      int cnt = send_cnt;
      while (cnt--) {
        auto msg = std::make_shared<myframe::Msg>(
          myframe::MAIN_CMD_ALL_USER_MOD_ADDR);
        msg->SetDst(myframe::MAIN_ADDR);
        auto resp = app->SendRequest(std::move(msg));
        if (resp == nullptr) {
          continue;
        }
        LOG(INFO) << "thread " << i
          << " resp(" << cnt << "): " << resp->GetData();
      }
      std::lock_guard<std::mutex> g(mtx);
      LOG(INFO) << "user thread " << i << " exit";
      ++exit_th_cnt;
      if (exit_th_cnt == th_cnt) {
        app->Quit();
      }
    }));
  }

  // 开始循环
  app->Exec();

  // 等待所有线程退出
  for (int i = 0; i < th_cnt; ++i) {
    if (th_vec[i].joinable()) {
      th_vec[i].join();
    }
  }
  return 0;
}
