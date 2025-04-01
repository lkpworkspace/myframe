/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
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

  int Init(const char* param) override {
    (void)param;
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
    LOG(INFO) << "recv msg: " << *msg;
  }

 private:
  std::atomic_bool th_run_{false};
  std::thread th_;
};

extern "C" MYFRAME_EXPORT std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "example_thread_quit") {
    return std::make_shared<ExampleThreadQuit>();
  }
  return nullptr;
}
