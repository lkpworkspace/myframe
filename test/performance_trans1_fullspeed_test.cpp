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

#include "myframe/log.h"
#include "myframe/common.h"
#include "myframe/msg.h"
#include "myframe/actor.h"
#include "myframe/mod_manager.h"
#include "myframe/app.h"

#include "performance_test_config.h"

class FullSpeedTransTest : public myframe::Actor {
 public:
  FullSpeedTransTest() : msg_(8192, 'z') {}

  int Init() override {
    LOG(INFO) << "init full speed trans ";
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
      LOG(INFO) << GetActorName() << ": full speed msg count " << cnt_;
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
      LOG(INFO) << "runing FullSpeedTransTest end";
      int sum = std::accumulate(msg_cnt_per_sec_list_.begin(),
        msg_cnt_per_sec_list_.end(), 0);
      int avg = sum / msg_cnt_per_sec_list_.size();
      LOG(INFO) << "1 actor fullspeed trans msg avg(cnt/sec): " << avg;
      std::sort(msg_cnt_per_sec_list_.begin(), msg_cnt_per_sec_list_.end());
      LOG(INFO) << "1 actor fullspeed trans msg 99(cnt/sec): "
        << msg_cnt_per_sec_list_[
            static_cast<size_t>(msg_cnt_per_sec_list_.size() * 0.99)];
      GetApp()->Quit();
    }
  }

 private:
  bool init_{false};
  int cnt_{0};
  std::chrono::high_resolution_clock::time_point begin_;
  std::chrono::high_resolution_clock::time_point last_;
  std::string msg_;
  std::vector<int> msg_cnt_per_sec_list_;
};

int main() {
  auto lib_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LIB_DIR).string();
  auto log_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LOG_DIR).string();

  myframe::InitLog(log_dir, "performance_trans1_fullspeed_test");

  auto app = std::make_shared<myframe::App>();
  myframe::Arguments args;
  args.SetStr(MYFRAME_KEY_SERVICE_LIB_DIR, lib_dir);
  args.SetInt(MYFRAME_KEY_THREAD_POOL_SIZE, 4);
  args.SetInt(MYFRAME_KEY_EVENT_CONNE_SIZE, 2);
  args.SetInt(MYFRAME_KEY_WARNING_MSG_SIZE, 10);
  args.SetInt(MYFRAME_KEY_PENDING_QUEUE_SIZE, -1);
  args.SetInt(MYFRAME_KEY_RUN_QUEUE_SIZE, 2);
  if (false == app->Init(args)) {
    LOG(ERROR) << "Init failed";
    return -1;
  }

  // mod manager
  auto& mod = app->GetModManager();

  // 1个actor消息吞吐量（测试时长1分钟，全速运行）
  mod->RegActor("FullSpeedTransTest", [](const std::string&) {
      return std::make_shared<FullSpeedTransTest>();
  });
  auto actor = mod->CreateActorInst("class", "FullSpeedTransTest", "0");
  app->AddActor(actor);

  return app->Exec();
}
