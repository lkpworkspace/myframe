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


class Trans10ActorCostTest : public myframe::Actor {
 public:
  Trans10ActorCostTest() : msg_(8192, 'y') {}

  int Init(const char* param) override {
    task_num_ = std::stoi(param);
    // 启动测试
    if (task_num_ == 0) {
      cost_us_list_.reserve(6000);
      auto mailbox = GetMailbox();
      mailbox->Send("actor.Trans10ActorCostTest.0",
        std::make_shared<myframe::Msg>(msg_));
    }
    LOG(INFO) << "init trans msg num " << task_num_;
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    (void)msg;
    if (!init_) {
      init_ = true;
      total_ = std::chrono::high_resolution_clock::now();
    }
    if (task_num_ == 0) {
      begin_ = std::chrono::high_resolution_clock::now();
    }
    if (task_num_ == 9) {
      auto now = std::chrono::high_resolution_clock::now();
      auto us =
          std::chrono::duration_cast<std::chrono::microseconds>(now - begin_)
              .count();
      LOG(INFO) << GetActorName() << " trans 10 actor msg cost(us) " << us;
      cost_us_list_.push_back(us);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      begin_ = std::chrono::high_resolution_clock::now();
    }
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::high_resolution_clock::now() - total_)
        .count();
    std::string next_actor_name =
        "actor.Trans10ActorCostTest." + std::to_string((task_num_ + 1) % 10);
    if (sec < 60) {
      auto mailbox = GetMailbox();
      mailbox->Send(next_actor_name, std::make_shared<myframe::Msg>(msg_));
    } else {
      LOG(INFO) << "runing Trans10ActorCostTest end";
      // 耗时
      //   平均值:
      //   99分位:
      int sum = std::accumulate(cost_us_list_.begin(), cost_us_list_.end(), 0);
      int avg = sum / cost_us_list_.size();
      LOG(INFO) << "trans 10 actor cnt: " << cost_us_list_.size();
      LOG(INFO) << "trans 10 actor avg(us): " << avg;
      std::sort(cost_us_list_.begin(), cost_us_list_.end());
      LOG(INFO) << "trans 10 actor 99(us): " <<
        cost_us_list_[
          static_cast<size_t>(cost_us_list_.size() * 0.99)];
      GetApp()->Quit();
    }
  }

 private:
  static bool init_;
  static std::chrono::high_resolution_clock::time_point total_;
  static std::chrono::high_resolution_clock::time_point begin_;
  static std::vector<int64_t> cost_us_list_;
  int task_num_{0};
  std::string msg_;
};
bool Trans10ActorCostTest::init_{false};
std::chrono::high_resolution_clock::time_point Trans10ActorCostTest::total_;
std::chrono::high_resolution_clock::time_point Trans10ActorCostTest::begin_;
std::vector<int64_t> Trans10ActorCostTest::cost_us_list_;


int main() {
  auto lib_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LIB_DIR).string();
  auto log_dir =
      myframe::Common::GetAbsolutePath(MYFRAME_LOG_DIR).string();

  myframe::InitLog(log_dir, "performance_trans10_cost_test");

  auto app = std::make_shared<myframe::App>();
  if (false == app->Init(lib_dir, 4)) {
    LOG(ERROR) << "Init failed";
    return -1;
  }

  // mod manager
  auto& mod = app->GetModManager();

  // 消息流转10个actor耗时（测试时长1分钟，每隔10毫秒发送1条消息）
  // 耗时
  //   平均值:
  //   99分位:
  mod->RegActor("Trans10ActorCostTest", [](const std::string&) {
      return std::make_shared<Trans10ActorCostTest>();
  });
  for (int i = 0; i < 10; ++i) {
      auto actor = mod->CreateActorInst("class", "Trans10ActorCostTest");
      app->AddActor(std::to_string(i), std::to_string(i), actor);
  }

  return app->Exec();
}
