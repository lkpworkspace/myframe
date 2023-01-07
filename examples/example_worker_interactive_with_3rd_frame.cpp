/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <chrono>
#include <thread>

#include <glog/logging.h>

#include "myframe/actor.h"
#include "myframe/common.h"
#include "myframe/msg.h"
#include "myframe/worker.h"

template <typename T>
class MyQueue final {
 public:
  MyQueue() { CreateSockPair(); }
  ~MyQueue() { CloseSockPair(); }

  int GetFd0() { return fd_pair_[0]; }
  int GetFd1() { return fd_pair_[1]; }

  void Push(std::shared_ptr<T> data) {
    data_ = data;
    char cmd_char = 'p';
    write(fd_pair_[0], &cmd_char, 1);
    read(fd_pair_[0], &cmd_char, 1);
  }

  std::shared_ptr<T> Pop() {
    std::shared_ptr<T> ret = nullptr;
    char cmd_char = '\0';
    read(fd_pair_[1], &cmd_char, 1);
    ret = data_;
    data_ = nullptr;
    write(fd_pair_[1], &cmd_char, 1);
    return ret;
  }

 private:
  bool CreateSockPair() {
    int res = -1;
    bool ret = true;

    res = socketpair(AF_UNIX, SOCK_DGRAM, 0, fd_pair_);
    if (res == -1) {
      LOG(ERROR) << "create sockpair failed";
      return false;
    }
    ret = myframe::Common::SetNonblockFd(fd_pair_[0], false);
    if (!ret) {
      LOG(ERROR) << "set sockpair[0] block failed";
      return ret;
    }
    ret = myframe::Common::SetNonblockFd(fd_pair_[1], false);
    if (!ret) {
      LOG(ERROR) << "set sockpair[1] block failed";
      return ret;
    }
    return ret;
  }
  void CloseSockPair() {
    if (-1 == close(fd_pair_[0])) {
      LOG(ERROR) << "close sockpair[0]: " << strerror(errno);
    }
    if (-1 == close(fd_pair_[1])) {
      LOG(ERROR) << "close sockpair[1]: " << strerror(errno);
    }
  }

  std::shared_ptr<T> data_;
  int fd_pair_[2] = {-1, -1};
};

/**
 * @brief 与其它程序或框架交互示例
 */
class ExampleWorkerInteractiveWith3rdFrame : public myframe::Worker {
 public:
  ExampleWorkerInteractiveWith3rdFrame() {}
  virtual ~ExampleWorkerInteractiveWith3rdFrame() {}

  void OnInit() override {
    // 线程_th通过MyQueue与myframe交互
    _th = std::thread([this]() {
      while (1) {
        _queue.Push(std::make_shared<std::string>("this is 3rd frame"));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
    });
    _th.detach();

    // 通知myframe该worker可以接收来自myframe的消息
    SendCmdToMain(myframe::WorkerCmd::WAIT_FOR_MSG);
  }

  void Run() override {
    bool has_main_msg = false;
    struct pollfd fds[] = {{GetWorkerFd(), POLLIN, 0},
                           {_queue.GetFd1(), POLLIN, 0}};
    // 等待来自queue或者myframe的消息
    poll(fds, 2, -1);
    if (fds[0].revents & POLLIN) {
      has_main_msg = true;
    }
    if (fds[1].revents & POLLIN) {
      auto data = _queue.Pop();
      // 可以将queue里的消息发给myfrmae的worker或actor
      // eg: Send("actor.xx.xx", std::make_shared<Msg>(data->c_str()));
      LOG(INFO) << "get msg from queue: " << data->c_str();
    }
    OnMainMsg(has_main_msg);
  }

  // 分发消息、处理来自myframe的消息
  void OnMainMsg(bool has_main_msg) {
    if (!has_main_msg) {
      SendCmdToMain(myframe::WorkerCmd::IDLE);
    }
    while (1) {
      myframe::WorkerCmd cmd;
      RecvCmdFromMain(&cmd);
      if (myframe::WorkerCmd::RUN == cmd) {
        return;
      }
      if (myframe::WorkerCmd::RUN_WITH_MSG == cmd) {
        ProcessMainMsg();
        SendCmdToMain(myframe::WorkerCmd::WAIT_FOR_MSG);
        if (has_main_msg) {
          return;
        }
      }
    }
  }

  void ProcessMainMsg() {
    while (RecvMsgListSize() > 0) {
      const auto& msg = GetRecvMsg();
      // ...
      LOG(INFO) << "get msg from main " << msg->GetData();
    }
  }

 private:
  std::thread _th;
  MyQueue<std::string> _queue;
};

class ExampleActorInteractiveWith3rdFrame : public myframe::Actor {
 public:
  int Init(const char* param) override {
    Timeout("100ms", 10);
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    if (msg->GetType() == "TIMER") {
      Send("worker.example_worker_interactive_with_3rd_frame.#1",
           std::make_shared<myframe::Msg>(
               "this is interactive_with_3rd_frame actor"));
      Timeout("1000ms", 100);
    }
  }
};

/* 创建worker实例函数 */
extern "C" std::shared_ptr<myframe::Worker> my_worker_create(
    const std::string& worker_name) {
  return std::make_shared<ExampleWorkerInteractiveWith3rdFrame>();
}

/* 创建actor实例函数 */
extern "C" std::shared_ptr<myframe::Actor> my_actor_create(
    const std::string& actor_name) {
  return std::make_shared<ExampleActorInteractiveWith3rdFrame>();
}
