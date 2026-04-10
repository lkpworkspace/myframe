/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
/*
示例概述：
  示范用户在Actor中创建的自定义线程如何优雅退出

创建对象：
  actor.ExampleThreadQuit.1

执行逻辑：
  初始化创建一个自定义线程每隔1秒发送消息给actor.ExampleThreadQuit.1
  在框架退出时会执行actor.ExampleThreadQuit.1析构函数，在析构函数中
  停止自定义线程运行并回收线程资源。
*/
#include <thread>
#include <chrono>
#include <memory>

#include "myframe/log.h"
#include "myframe/msg.h"
#include "myframe/actor.h"
#include "myframe/app.h"

class ExampleThreadQuit : public myframe::Actor {
 public:
  virtual ~ExampleThreadQuit() {
    LOG(INFO) << "thread quit example quiting...";
    th_run_.store(false);
    if (th_.joinable()) {
      th_.join();
    }
    LOG(INFO) << "thread quit example quit";
  }

  int Init() override {
    th_run_.store(true);
    th_ = std::thread([this](){
      while (th_run_.load()) {
        LOG(INFO) << "thread quit example runing...";
        auto app = GetApp();
        auto msg = std::make_shared<myframe::Msg>();
        msg->SetDst(GetActorName());
        msg->SetData("thread quit example");
        msg->SetType("TEXT");
        app->Send(std::move(msg));
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    });
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    LOG(INFO) << *msg << ", data: " << msg->GetData();
  }

 private:
  std::atomic_bool th_run_{false};
  std::thread th_;
};

extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "ExampleThreadQuit") {
    return std::make_shared<ExampleThreadQuit>();
  }
  return nullptr;
}
