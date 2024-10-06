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

class TransMsgCostTest : public myframe::Actor {
 public:
  TransMsgCostTest() : msg_(8192, 'x') {}

  int Init(const char* param) override {
    (void)param;
    LOG(INFO) << "runing TransMsgCostTest...";
    cost_us_list_.reserve(6000);
    last_ = std::chrono::high_resolution_clock::now();
    begin_ = std::chrono::high_resolution_clock::now();
    auto mailbox = GetMailbox();
    mailbox->Send(GetActorName(), std::make_shared<myframe::Msg>(msg_));
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    (void)msg;
    auto now = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(now - last_)
                  .count();
    LOG(INFO) << GetActorName() << " trans msg cost(us) " << us;
    cost_us_list_.push_back(us);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    last_ = std::chrono::high_resolution_clock::now();

    auto sec = std::chrono::duration_cast<std::chrono::seconds>(last_ - begin_)
                   .count();
    if (sec < 60) {
      auto mailbox = GetMailbox();
      mailbox->Send(GetActorName(), std::make_shared<myframe::Msg>(msg_));
    } else {
      LOG(INFO) << "runing TransMsgCostTest end";
      // 耗时
      //   平均值:
      //   99分位:
      int sum = std::accumulate(cost_us_list_.begin(), cost_us_list_.end(), 0);
      int avg = sum / cost_us_list_.size();
      LOG(INFO) << "trans 1 actor cnt: " << cost_us_list_.size();
      LOG(INFO) << "trans 1 actor avg(us): " << avg;
      std::sort(cost_us_list_.begin(), cost_us_list_.end());
      LOG(INFO) << "trans 1 actor 99(us): " <<
        cost_us_list_[
            static_cast<size_t>(cost_us_list_.size() * 0.99)];
      GetApp()->Quit();
    }
  }

 private:
  std::chrono::high_resolution_clock::time_point begin_;
  std::chrono::high_resolution_clock::time_point last_;
  std::string msg_;
  std::vector<int64_t> cost_us_list_;
};

int main() {
  auto lib_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LIB_DIR).string();
  auto log_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LOG_DIR).string();

  myframe::InitLog(log_dir, "performance_trans1_cost_test");

  auto app = std::make_shared<myframe::App>();
  if (false == app->Init(lib_dir, 4)) {
    LOG(ERROR) << "Init failed";
    return -1;
  }

  // mod manager
  auto& mod = app->GetModManager();

  // 发送单条消息耗时（测试时长1分钟，每隔10毫秒发送1条消息）
  // 耗时
  //   平均值:
  //   99分位:
  mod->RegActor("TransMsgCostTest", [](const std::string&) {
    return std::make_shared<TransMsgCostTest>();
  });
  auto actor = mod->CreateActorInst("class", "TransMsgCostTest");
  app->AddActor("#1", "", actor);

  return app->Exec();
}
