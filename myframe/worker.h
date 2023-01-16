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

#include "myframe/event.h"
#include "myframe/mailbox.h"
#include "myframe/cmd_channel.h"

namespace myframe {

enum class WorkerCtrlOwner : int {
  kMain,
  kWorker,
};

class Worker : public Event {
  friend class App;
  friend class ModLib;
  friend class ModManager;
  friend class WorkerManager;

 public:
  Worker();
  virtual ~Worker();

  Mailbox* GetMailbox();

  CmdChannel* GetCmdChannel();

  ////////////////////////////// thread 相关函数
  virtual void OnInit() {}
  virtual void Run() = 0;
  virtual void OnExit() {}
  void Start();
  void Stop();
  bool IsRuning() { return runing_.load(); }
  pthread_t GetPosixThreadId() { return th_.native_handle(); }

  ////////////////////////////// event 相关函数
  int GetFd() const override;

  /// 分发消息并立即返回
  int DispatchMsg();
  /// 分发消息并等待回复消息
  int DispatchAndWaitMsg();

  const std::string GetWorkerName() const;
  const std::string& GetModName() const;
  const std::string& GetTypeName() const;
  const std::string& GetInstName() const;
  const Json::Value* GetConfig() { return &config_; }

 private:
  static void ListenThread(std::shared_ptr<Worker> w);
  void Initialize();

  ////////////////////////////// recv cache list method
  int CacheSize() const;
  std::list<std::shared_ptr<Msg>>* GetCache();
  void Cache(std::shared_ptr<Msg> msg);
  void Cache(std::list<std::shared_ptr<Msg>>* msg_list);

  ////////////////////////////// 线程交互控制flag函数
  void SetCtrlOwnerFlag(WorkerCtrlOwner owner) { ctrl_owner_ = owner; }
  WorkerCtrlOwner GetOwner() const { return ctrl_owner_; }
  void SetWaitMsgQueueFlag(bool in_wait_msg_queue) {
    in_msg_wait_queue_ = in_wait_msg_queue;
  }
  bool IsInWaitMsgQueue() { return in_msg_wait_queue_; }

  ////////////////////////////// worker name
  void SetModName(const std::string& name);
  void SetTypeName(const std::string& name);
  void SetInstName(const std::string& name);
  void SetConfig(const Json::Value& conf) { config_ = conf; }

  /// worker name
  std::string mod_name_;
  std::string worker_name_;
  std::string inst_name_;
  Json::Value config_{ Json::Value::null };
  /// state flag
  std::atomic_bool runing_;
  WorkerCtrlOwner ctrl_owner_{ WorkerCtrlOwner::kWorker };
  bool in_msg_wait_queue_{ false };
  /// recv cache list
  std::list<std::shared_ptr<Msg>> cache_;
  /// mailbox
  Mailbox mailbox_;
  /// cmd channel
  CmdChannel cmd_channel_;
  /// thread
  std::thread th_;
};

}  // namespace myframe

extern "C" {
typedef std::shared_ptr<myframe::Worker> (*my_worker_create_func)(
    const std::string&);
}  // extern "C"
