/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <list>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#include "myframe/macros.h"
#include "myframe/event.h"

namespace myframe {

class Msg;
class EventManager;
class WorkerContext;
class WorkerContextManager final {
 public:
  WorkerContextManager(std::shared_ptr<EventManager>);
  virtual ~WorkerContextManager();

  bool Init(int warning_msg_size = 10);

  int WorkerSize();

  bool Add(std::shared_ptr<WorkerContext> worker);
  void Del(std::shared_ptr<WorkerContext> worker);

  // 内置工作线程池
  int IdleWorkerSize();
  std::shared_ptr<WorkerContext> FrontIdleWorker();
  void PopFrontIdleWorker();
  void PushBackIdleWorker(std::shared_ptr<WorkerContext> worker);

  // 用户工作线程
  void PushWaitWorker(std::shared_ptr<WorkerContext> worker);
  void WeakupWorker();
  void DispatchWorkerMsg(
    std::shared_ptr<Msg> msg,
    const std::string& dst = "");

  std::vector<std::string> GetAllUserWorkerAddr();

  // 停止工作线程
  void StopAllWorker();
  void WaitAllWorkerQuit();
  void ClearStopWorker();

 private:
  std::atomic<std::size_t> warning_msg_size_{10};
  /// 工作线程数(包含用户线程)
  std::atomic_int cur_worker_count_{0};
  /// 读写锁
  std::shared_mutex rw_;
  /// 空闲线程链表
  std::list<std::weak_ptr<WorkerContext>> idle_workers_ctx_;
  /// 有消息user线程
  std::list<std::weak_ptr<WorkerContext>> weakup_workers_ctx_;
  /// 停止的线程列表
  std::list<std::shared_ptr<WorkerContext>> stoped_workers_ctx_;
  /// 事件管理对象
  std::shared_ptr<EventManager> ev_mgr_;

  DISALLOW_COPY_AND_ASSIGN(WorkerContextManager)
};

}  // namespace myframe
