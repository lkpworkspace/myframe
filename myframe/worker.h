/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <pthread.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <atomic>
#include <list>
#include <memory>
#include <string>
#include <thread>

#include "myframe/event.h"
#include "myframe/msg.h"

namespace myframe {

/**
 * @brief worker与main交互命令
 *  QUIT main主动发起的退出命令
 *  IDLE与RUN为一对请求回复命令
 *  WAIT_FOR_MSG与RUN_WITH_MSG为一对请求回复命令
 */
enum class WorkerCmd : char {
  QUIT = 'q',          ///< worker退出(worker发送的指令)
  IDLE = 'i',          ///< worker空闲(worker发送的指令)
  WAIT_FOR_MSG = 'w',  ///< worker等待消息(worker发送的指令)
  RUN = 'r',           ///< worker运行(main回复的指令)
  RUN_WITH_MSG = 'm',  ///< worker运行(main回复的指令)
};

enum class WorkerCtrlOwner : int {
  MAIN,
  WORKER,
};

class Worker : public Event {
  friend class App;
  friend class ModLib;
  friend class ModManager;
  friend class WorkerManager;

 public:
  Worker();
  virtual ~Worker();

  ////////////////////////////// thread 相关函数
  virtual void OnInit() {}
  virtual void Run() = 0;
  virtual void OnExit() {}
  void Start();
  void Stop();
  bool IsRuning() { return runing_; }
  pthread_t GetPosixThreadId() { return th_.native_handle(); }

  ////////////////////////////// event 相关函数
  int GetFd() override { return sock_pair_[1]; }
  unsigned int ListenEpollEventType() override { return EPOLLIN; }
  void RetEpollEventType(uint32_t ev) override { ev = ev; }

  ////////////////////////////// 接收worker/actor消息
  int RecvMsgListSize() { return que_.size(); }
  const std::shared_ptr<const Msg> GetRecvMsg();

  ////////////////////////////// 发送消息相关函数
  int SendMsgListSize() { return send_.size(); }
  void SendMsg(const std::string& dst, std::shared_ptr<Msg> msg);
  void SendMsg(const std::string& dst, std::any data);
  void PushSendMsgList(std::list<std::shared_ptr<Msg>>* msg_list);

  ////////////////////////////// 接收/发送主线程控制消息
  /// 不建议使用,除非你知道你在做什么
  int RecvCmdFromMain(WorkerCmd* cmd, int timeout_ms = -1);
  int SendCmdToMain(const WorkerCmd& cmd);
  /// 分发消息并立即返回
  int DispatchMsg();
  /// 分发消息并等待回复消息
  int DispatchAndWaitMsg();

  /// worker fd
  int GetWorkerFd() { return sock_pair_[0]; }

  const std::string& GetModName() const { return mod_name_; }
  const std::string& GetTypeName() const { return worker_name_; }
  const std::string& GetInstName() const { return inst_name_; }
  const std::string GetWorkerName() const;

 private:
  static void ListenThread(std::shared_ptr<Worker> w);

  ////////////////////////////// 线程间通信相关函数
  int SendCmdToWorker(const WorkerCmd& cmd);
  int RecvCmdFromWorker(WorkerCmd* cmd);

  ////////////////////////////// 线程交互控制flag函数
  void SetCtrlOwnerFlag(WorkerCtrlOwner owner) { ctrl_owner_ = owner; }
  WorkerCtrlOwner GetOwner() const { return ctrl_owner_; }
  void SetWaitMsgQueueFlag(bool in_wait_msg_queue) {
    in_msg_wait_queue_ = in_wait_msg_queue;
  }
  bool IsInWaitMsgQueue() { return in_msg_wait_queue_; }

  ////////////////////////////// worker name
  void SetModName(const std::string& name) { mod_name_ = name; }
  void SetTypeName(const std::string& name) { worker_name_ = name; }
  void SetInstName(const std::string& name) { inst_name_ = name; }

  bool CreateSockPair();
  void CloseSockPair();

  /// worker name
  std::string mod_name_;
  std::string worker_name_;
  std::string inst_name_;
  /// idx: 0 used by WorkerCommon, 1 used by app
  int sock_pair_[2];
  /// state
  std::atomic_bool runing_;
  WorkerCtrlOwner ctrl_owner_{ WorkerCtrlOwner::WORKER };
  bool in_msg_wait_queue_{false};
  /// 接收消息队列
  std::list<std::shared_ptr<Msg>> recv_;
  /// 运行时消息队列
  std::list<std::shared_ptr<Msg>> que_;
  /// 发送消息队列
  std::list<std::shared_ptr<Msg>> send_;
  /// thread
  std::thread th_;
};

}  // namespace myframe

extern "C" {
typedef std::shared_ptr<myframe::Worker> (*my_worker_create_func)(
    const std::string&);
}  // extern "C"
