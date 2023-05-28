/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include <poll.h>

#include <chrono>
#include <thread>

#include <glog/logging.h>

#include "myframe/common.h"
#include "myframe/msg.h"
#include "myframe/cmd_channel.h"
#include "myframe/actor.h"
#include "myframe/worker.h"

template <typename T>
class MyQueue final {
 public:
  MyQueue() = default;
  ~MyQueue() = default;

  int GetFd0() { return cmd_channel_.GetOwnerFd(); }
  int GetFd1() { return cmd_channel_.GetMainFd(); }

  void Push(std::shared_ptr<T> data) {
    data_ = data;
    myframe::Cmd cmd = myframe::Cmd::kRun;
    cmd_channel_.SendToOwner(cmd);
    cmd_channel_.RecvFromOwner(&cmd);
  }

  std::shared_ptr<T> Pop() {
    std::shared_ptr<T> ret = nullptr;
    myframe::Cmd cmd = myframe::Cmd::kRun;
    cmd_channel_.RecvFromMain(&cmd);
    ret = data_;
    data_ = nullptr;
    cmd_channel_.SendToMain(myframe::Cmd::kIdle);
    return ret;
  }

 private:
  std::shared_ptr<T> data_;
  myframe::CmdChannel cmd_channel_;
};

/**
 * @brief 与其它程序或框架交互示例
 */
class ExampleWorkerInteractiveWith3rdFrame : public myframe::Worker {
 public:
  ExampleWorkerInteractiveWith3rdFrame() {}
  virtual ~ExampleWorkerInteractiveWith3rdFrame() {}

  void Init() override {
    quit_.store(false);
    // 线程th_通过MyQueue与myframe交互
    th_ = std::thread([this]() {
      while (!quit_.load()) {
        seq_num_++;
        LOG(INFO) << "3rd frame pub " << seq_num_;
        queue_.Push(std::make_shared<std::string>(std::to_string(seq_num_)));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
    });

    // 通知myframe该worker可以接收来自myframe的消息
    GetCmdChannel()->SendToMain(myframe::Cmd::kWaitForMsg);
  }

  void Run() override {
    auto cmd_channel = GetCmdChannel();
    struct pollfd fds[] = {
      {cmd_channel->GetOwnerFd(), POLLIN, 0},
      {queue_.GetFd0(), POLLIN, 0}};
    // 等待来自queue或者myframe的消息
    int ret = poll(fds, 2, -1);
    if (ret < 0) {
      LOG(ERROR) << "poll error, " << strerror(errno);
      return;
    }
    for (std::size_t i = 0; i < 2; ++i) {
      if (!(fds[i].revents & POLLIN)) {
        continue;
      }
      if (i == 0) {
        OnMainMsg();
      } else if (i == 1) {
        auto data = queue_.Pop();
        // 可以将queue里的消息发给myfrmae的worker或actor
        // eg: Send("actor.xx.xx", std::make_shared<Msg>(data->c_str()));
        LOG(INFO) << "get 3rd frame: " << data->c_str();
        cmd_channel->SendToMain(myframe::Cmd::kIdle);
      }
    }
  }

  void Exit() override {
    if (th_.joinable()) {
      th_.join();
    }
  }

  // 分发消息、处理来自myframe的消息
  void OnMainMsg() {
    auto cmd_channel = GetCmdChannel();
    myframe::Cmd cmd;
    cmd_channel->RecvFromMain(&cmd);
    if (cmd == myframe::Cmd::kRun) {
      // do nothing
      return;
    } else if (cmd == myframe::Cmd::kRunWithMsg) {
      ProcessMainMsg();
      cmd_channel->SendToMain(myframe::Cmd::kWaitForMsg);
    } else if (cmd == myframe::Cmd::kQuit) {
      quit_.store(true);
      Stop();
    }
  }

  void ProcessMainMsg() {
    auto mailbox = GetMailbox();
    while (!mailbox->RecvEmpty()) {
      const auto& msg = mailbox->PopRecv();
      // ...
      LOG(INFO) << "get main " << msg->GetData();
    }
  }

 private:
  int seq_num_{0};
  std::atomic_bool quit_{true};
  std::thread th_;
  MyQueue<std::string> queue_;
};

class ExampleActorInteractiveWith3rdFrame : public myframe::Actor {
 public:
  int Init(const char* param) override {
    (void)param;
    Timeout("100ms", 10);
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    if (msg->GetType() == "TIMER") {
      auto mailbox = GetMailbox();
      seq_num_++;
      LOG(INFO) << "actor pub " << seq_num_;
      mailbox->Send(
        "worker.example_worker_interactive_with_3rd_frame.#1",
        std::make_shared<myframe::Msg>(std::to_string(seq_num_)));
      Timeout("100ms", 10);
    }
  }
 private:
  int seq_num_{0};
};

/* 创建worker实例函数 */
extern "C" std::shared_ptr<myframe::Worker> worker_create(
    const std::string& worker_name) {
  if (worker_name == "example_worker_interactive_with_3rd_frame") {
    return std::make_shared<ExampleWorkerInteractiveWith3rdFrame>();
  }
  return nullptr;
}

/* 创建actor实例函数 */
extern "C" std::shared_ptr<myframe::Actor> actor_create(
    const std::string& actor_name) {
  if (actor_name == "example_actor_interactive_with_3rd_frame") {
    return std::make_shared<ExampleActorInteractiveWith3rdFrame>();
  }
  return nullptr;
}
