/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <atomic>
#include <list>
#include <memory>
#include <string>
#include <thread>

#include <jsoncpp/json/json.h>

#include "myframe/macros.h"
#include "myframe/event.h"
#include "myframe/mailbox.h"
#include "myframe/cmd_channel.h"

namespace myframe {

enum class WorkerCtrlOwner : int {
  kMain,
  kWorker,
};

class App;
class Worker;
class WorkerContext final : public Event {
 public:
  WorkerContext(std::shared_ptr<App> app, std::shared_ptr<Worker> worker);
  virtual ~WorkerContext();

  /// thread 相关函数
  void Start();
  void Stop();
  bool IsRuning() { return runing_.load(); }
  pthread_t GetPosixThreadId() { return th_.native_handle(); }

  /// event 相关函数
  int GetFd() const override;
  EventType GetType() override;

  Mailbox* GetMailbox();

  CmdChannel* GetCmdChannel();

  template<typename T>
  std::shared_ptr<T> GetWorker() const {
    return std::dynamic_pointer_cast<T>(worker_);
  }

  /// recv cache list method
  std::size_t CacheSize() const;
  std::list<std::shared_ptr<Msg>>* GetCache();
  void Cache(std::shared_ptr<Msg> msg);
  void Cache(std::list<std::shared_ptr<Msg>>* msg_list);

  /// 线程交互控制flag函数
  void SetCtrlOwnerFlag(WorkerCtrlOwner owner) {
    ctrl_owner_ = owner;
  }
  WorkerCtrlOwner GetOwner() const {
    return ctrl_owner_;
  }
  void SetWaitMsgQueueFlag(bool in_wait_msg_queue) {
    in_msg_wait_queue_ = in_wait_msg_queue;
  }
  bool IsInWaitMsgQueue() {
    return in_msg_wait_queue_;
  }

 private:
  static void ListenThread(std::shared_ptr<WorkerContext> w);
  void Initialize();

  /// state flag
  std::atomic_bool runing_;
  WorkerCtrlOwner ctrl_owner_{ WorkerCtrlOwner::kWorker };
  bool in_msg_wait_queue_{ false };

  /// worker
  std::shared_ptr<Worker> worker_;
  std::weak_ptr<App> app_;

  /// recv cache list
  std::list<std::shared_ptr<Msg>> cache_;

  /// mailbox
  Mailbox mailbox_;

  /// cmd channel
  CmdChannel cmd_channel_;

  /// thread
  std::thread th_;

  DISALLOW_COPY_AND_ASSIGN(WorkerContext)
};

std::ostream& operator<<(std::ostream& out, WorkerContext& ctx);

}  // namespace myframe
