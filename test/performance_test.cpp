/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include <list>
#include <memory>
#include <thread>

#include <gtest/gtest.h>
#include <glog/logging.h>

#include "myframe/actor.h"
#include "myframe/app.h"
#include "myframe/flags.h"
#include "myframe/log.h"
#include "myframe/mod_manager.h"
#include "myframe/msg.h"

class EchoActorTest : public myframe::Actor {
 public:
  int Init(const char* param) override {
    LOG(INFO) << "init EchoActorTest";
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    LOG(INFO) << "recv " << msg->GetSrc() << ":" << msg->GetData();
    auto re = std::make_shared<myframe::Msg>("resp:" + std::to_string(seq_++));
    auto mailbox = GetMailbox();
    mailbox->Send(msg->GetSrc(), re);
  }

 private:
  int seq_{0};
};

class TransMsgCostTest : public myframe::Actor {
 public:
  TransMsgCostTest() : msg_(8192, 'x') {}

  int Init(const char* param) override {
    LOG(INFO) << "begin runing TransMsgCostTest";
    last_ = std::chrono::high_resolution_clock::now();
    begin_ = std::chrono::high_resolution_clock::now();
    auto mailbox = GetMailbox();
    mailbox->Send(GetActorName(), std::make_shared<myframe::Msg>(msg_));
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    auto now = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(now - last_)
                  .count();
    LOG(INFO) << GetActorName() << " trans msg cost(us) " << us;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    last_ = std::chrono::high_resolution_clock::now();

    auto sec = std::chrono::duration_cast<std::chrono::seconds>(last_ - begin_)
                   .count();
    if (sec < 60) {
      auto mailbox = GetMailbox();
      mailbox->Send(GetActorName(), std::make_shared<myframe::Msg>(msg_));
    } else {
      LOG(INFO) << "runing next test...";
      auto mailbox = GetMailbox();
      mailbox->Send("actor.Trans10ActorCostTest.0",
           std::make_shared<myframe::Msg>(msg_));
    }
  }

 private:
  std::chrono::high_resolution_clock::time_point begin_;
  std::chrono::high_resolution_clock::time_point last_;
  std::string msg_;
};

class Trans10ActorCostTest : public myframe::Actor {
 public:
  Trans10ActorCostTest() : msg_(8192, 'y') {}

  int Init(const char* param) override {
    task_num_ = std::stoi(param);
    LOG(INFO) << "init trans msg num " << task_num_;
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
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
      LOG(INFO) << "runing next test...";
      auto mailbox = GetMailbox();
      mailbox->Send("actor.FullSpeedTransTest.0",
        std::make_shared<myframe::Msg>(msg_));
    }
  }

 private:
  static bool init_;
  static std::chrono::high_resolution_clock::time_point total_;
  static std::chrono::high_resolution_clock::time_point begin_;
  int task_num_{0};
  std::string msg_;
};
bool Trans10ActorCostTest::init_{false};
std::chrono::high_resolution_clock::time_point Trans10ActorCostTest::total_;
std::chrono::high_resolution_clock::time_point Trans10ActorCostTest::begin_;

class FullSpeedTransTest : public myframe::Actor {
 public:
  FullSpeedTransTest() : msg_(8192, 'z') {}

  int Init(const char* param) override {
    LOG(INFO) << "init full speed trans ";
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
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
      LOG(INFO) << "runing next test...";
      for (int i = 0; i < 20; ++i) {
        std::string name =
            "actor.FullSpeed20ActorTransTest." + std::to_string(i);
        auto mailbox = GetMailbox();
        mailbox->Send(name, std::make_shared<myframe::Msg>(msg_));
      }
    }
  }

 private:
  bool init_{false};
  int cnt_{0};
  std::chrono::high_resolution_clock::time_point begin_;
  std::chrono::high_resolution_clock::time_point last_;
  std::string msg_;
};

class FullSpeed20ActorTransTest : public myframe::Actor {
 public:
  FullSpeed20ActorTransTest() : msg_(8192, 'j') {}

  int Init(const char* param) override {
    LOG(INFO) << "init full speed 20 actor trans ";
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
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
      LOG(INFO) << GetActorName() << ": full speed 20 actor msg count " << cnt_;
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
        is_send_ = true;
        LOG(INFO) << "runing next test...";
        for (int i = 0; i < 100; ++i) {
          std::string name =
              "actor.FullSpeed100ActorTransTest." + std::to_string(i);
          auto mailbox = GetMailbox();
          mailbox->Send(name, std::make_shared<myframe::Msg>(msg_));
        }
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
};
std::atomic_bool FullSpeed20ActorTransTest::is_send_{false};

class FullSpeed100ActorTransTest : public myframe::Actor {
 public:
  FullSpeed100ActorTransTest() : msg_(8192, 'k') {}

  int Init(const char* param) override {
    LOG(INFO) << "init full speed 100 actor trans ";
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
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
      cnt_ = 0;
      last_ = std::chrono::high_resolution_clock::now();
    }
    auto sec = std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::high_resolution_clock::now() - begin_)
                   .count();
    if (sec < 60) {
      auto mailbox = GetMailbox();
      mailbox->Send(GetActorName(), std::make_shared<myframe::Msg>());
    }
  }

 private:
  bool init_{false};
  int cnt_{0};
  std::chrono::high_resolution_clock::time_point begin_;
  std::chrono::high_resolution_clock::time_point last_;
  std::string msg_;
};

TEST(App, performance_test) {
  myframe::InitLog();

  myframe::FLAGS_myframe_worker_count = 4;

  auto app = std::make_shared<myframe::App>();
  if (false == app->Init()) {
    LOG(ERROR) << "Init failed";
    return;
  }

  // mod manager
  auto& mod = app->GetModManager();

  // 注册echo Actor
  {
    mod->RegActor("EchoActorTest", [](const std::string&) {
      return std::make_shared<EchoActorTest>();
    });
    auto actor = mod->CreateActorInst("class", "EchoActorTest");
    app->AddActor("1", "", actor);
  }

  // 发送单条消息耗时（测试时长1分钟，每隔10毫秒发送1条消息）
  // 耗时
  //   平均值:
  //   99分位:
  {
    mod->RegActor("TransMsgCostTest", [](const std::string&) {
      return std::make_shared<TransMsgCostTest>();
    });
    auto actor = mod->CreateActorInst("class", "TransMsgCostTest");
    app->AddActor("#1", "", actor);
  }

  // 消息流转10个actor耗时（测试时长1分钟，每隔10毫秒发送1条消息）
  // 耗时
  //   平均值:
  //   99分位:
  {
    mod->RegActor("Trans10ActorCostTest", [](const std::string&) {
      return std::make_shared<Trans10ActorCostTest>();
    });
    for (int i = 0; i < 10; ++i) {
      auto actor = mod->CreateActorInst("class", "Trans10ActorCostTest");
      app->AddActor(std::to_string(i), std::to_string(i), actor);
    }
  }

  // 1个actor消息吞吐量（测试时长1分钟，全速运行）
  // 消息个数:
  // 消息大小:
  {
    mod->RegActor("FullSpeedTransTest", [](const std::string&) {
      return std::make_shared<FullSpeedTransTest>();
    });
    auto actor = mod->CreateActorInst("class", "FullSpeedTransTest");
    app->AddActor("0", std::to_string(0), actor);
  }

  // 20个actor消息吞吐量（测试时长1分钟，全速运行）
  // 消息个数:
  // 消息大小:
  {
    mod->RegActor("FullSpeed20ActorTransTest", [](const std::string&) {
      return std::make_shared<FullSpeed20ActorTransTest>();
    });
    for (int i = 0; i < 20; ++i) {
      auto actor = mod->CreateActorInst("class", "FullSpeed20ActorTransTest");
      app->AddActor(std::to_string(i), std::to_string(i), actor);
    }
  }

  // 100个actor消息吞吐量（测试时长1分钟，全速运行）
  // 消息个数:
  // 消息大小:
  {
    mod->RegActor("FullSpeed100ActorTransTest", [](const std::string&) {
      return std::make_shared<FullSpeed100ActorTransTest>();
    });
    for (int i = 0; i < 100; ++i) {
      auto actor = mod->CreateActorInst("class", "FullSpeed100ActorTransTest");
      app->AddActor(std::to_string(i), std::to_string(i), actor);
    }
  }

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
  });

  app->Exec();
}
