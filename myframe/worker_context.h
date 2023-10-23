/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <atomic>
#include <list>
#include <memory>
#include <string>
#include <thread>

#include <json/json.h>

#include "myframe/macros.h"
#include "myframe/event.h"
#include "myframe/mailbox.h"
#include "myframe/cmd_channel.h"

namespace myframe {

class App;
class Worker;
class WorkerContext final : public Event {
 public:
  enum class CtrlOwner : int {
    kMain,
    kWorker,
  };

  WorkerContext(
    std::shared_ptr<App> app,
    std::shared_ptr<Worker> worker,
    std::shared_ptr<Poller> poller);
  virtual ~WorkerContext();

  /// thread 相关函数
  void Start();
  void Stop();
  void Join();
  bool IsRuning() { return runing_.load(); }
  std::thread::id GetThreadId() { return th_.get_id(); }

  /// event 相关函数
  ev_handle_t GetHandle() const override;
  Event::Type GetType() const override;
  std::string GetName() const override;

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
  void SetCtrlOwnerFlag(CtrlOwner owner) {
    ctrl_owner_ = owner;
  }
  CtrlOwner GetOwner() const {
    return ctrl_owner_;
  }
  void SetWaitMsgQueueFlag(bool in_wait_msg_queue) {
    in_msg_wait_queue_ = in_wait_msg_queue;
  }
  bool IsInWaitMsgQueue() {
    return in_msg_wait_queue_;
  }

  std::shared_ptr<App> GetApp();

 private:
  void ListenThread();
  void Initialize();

  /// state flag
  std::atomic_bool runing_;
  CtrlOwner ctrl_owner_{ CtrlOwner::kWorker };
  bool in_msg_wait_queue_{ false };

  /// worker
  std::shared_ptr<Worker> worker_;
  std::weak_ptr<App> app_;

  /// recv cache list
  std::list<std::shared_ptr<Msg>> cache_;

  /// mailbox
  Mailbox mailbox_;

  /// cmd channel
  std::shared_ptr<CmdChannel> cmd_channel_;

  /// thread
  std::thread th_;

  DISALLOW_COPY_AND_ASSIGN(WorkerContext)
};

std::ostream& operator<<(std::ostream& out, WorkerContext& ctx);

}  // namespace myframe
