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

class FullSpeed100ActorTransTest : public myframe::Actor {
 public:
  FullSpeed100ActorTransTest() : msg_(8192, 'k') {}

  int Init(const char* param) override {
    (void)param;
    LOG(INFO) << "init full speed 100 actor trans ";
    // 启动测试
    msg_cnt_per_sec_list_.reserve(64);
    auto mailbox = GetMailbox();
    mailbox->Send(GetActorName(), std::make_shared<myframe::Msg>(msg_));
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    (void)msg;
    if (!init_) {
      init_ = true;
      begin_ = std::chrono::high_resolution_clock::now();
      last_ = std::chrono::high_resolution_clock::now();
    }
    auto now = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(now - last_)
                  .count();
    cnt_++;
    if (us / 1000.0 > 1000.0) {
      LOG(INFO) << GetActorName() << ": full speed 100 actor msg count "
                << cnt_;
      msg_cnt_per_sec_list_.push_back(cnt_);
      cnt_ = 0;
      last_ = std::chrono::high_resolution_clock::now();
    }
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::high_resolution_clock::now() - begin_)
                   .count();
    if (sec < 60) {
      auto mailbox = GetMailbox();
      mailbox->Send(GetActorName(), std::make_shared<myframe::Msg>());
    } else {
      if (!is_send_) {
        is_send_.store(true);
        LOG(INFO) << "runing FullSpeed100ActorTransTest end";
        int sum = std::accumulate(msg_cnt_per_sec_list_.begin(),
          msg_cnt_per_sec_list_.end(), 0);
        int avg = sum / msg_cnt_per_sec_list_.size();
        LOG(INFO) << "100 actor fullspeed trans msg avg(cnt/sec): " << avg;
        std::sort(msg_cnt_per_sec_list_.begin(), msg_cnt_per_sec_list_.end());
        LOG(INFO) << "100 actor fullspeed trans msg 99(cnt/sec): "
          << msg_cnt_per_sec_list_[msg_cnt_per_sec_list_.size() * 0.99];
        GetApp()->Quit();
      }
    }
  }

 private:
  static std::atomic_bool is_send_;
  bool init_{false};
  int cnt_{0};
  std::chrono::high_resolution_clock::time_point begin_;
  std::chrono::high_resolution_clock::time_point last_;
  std::string msg_;
  std::vector<int> msg_cnt_per_sec_list_;
};
std::atomic_bool FullSpeed100ActorTransTest::is_send_{false};

int main() {
  auto lib_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LIB_DIR);
  auto log_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LOG_DIR);

  myframe::InitLog(log_dir, "performance_trans100_fullspeed_test");

  auto app = std::make_shared<myframe::App>();
  if (false == app->Init(lib_dir, 4)) {
    LOG(ERROR) << "Init failed";
    return -1;
  }

  // mod manager
  auto& mod = app->GetModManager();

  // 100个actor消息吞吐量（测试时长1分钟，全速运行）
  mod->RegActor("FullSpeed100ActorTransTest", [](const std::string&) {
    return std::make_shared<FullSpeed100ActorTransTest>();
  });
  for (int i = 0; i < 100; ++i) {
    auto actor = mod->CreateActorInst("class", "FullSpeed100ActorTransTest");
    app->AddActor(std::to_string(i), std::to_string(i), actor);
  }

  LOG(INFO) << "FullSpeed100ActorTransTest begin...";
  return app->Exec();
}
