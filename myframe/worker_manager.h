/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <pthread.h>

#include <atomic>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

#include "myframe/macros.h"

namespace myframe {

class Msg;
class Worker;
class WorkerManager final {
 public:
  WorkerManager();
  virtual ~WorkerManager();

  int WorkerSize();

  std::shared_ptr<Worker> Get(int fd);
  std::shared_ptr<Worker> Get(const std::string&);
  bool Add(std::shared_ptr<Worker> worker);
  void Del(std::shared_ptr<Worker> worker);

  int IdleWorkerSize();
  std::shared_ptr<Worker> FrontIdleWorker();
  void PopFrontIdleWorker();
  void PushBackIdleWorker(std::shared_ptr<Worker> worker);

  void PushWaitWorker(std::shared_ptr<Worker> worker);
  void WeakupWorker();

  void DispatchWorkerMsg(std::shared_ptr<Msg> msg);

 private:
  /// 工作线程数(包含用户线程)
  std::atomic_int cur_worker_count_ = {0};
  /// 读写锁
  pthread_rwlock_t rw_;
  /// 空闲线程链表
  std::list<std::weak_ptr<Worker>> idle_workers_;
  /// 有消息user线程
  std::list<std::weak_ptr<Worker>> weakup_workers_;
  /// name/handle 映射表
  std::unordered_map<std::string, int> name_handle_map_;
  /// handle/worker 映射表
  std::unordered_map<int, std::shared_ptr<Worker>> workers_;

  DISALLOW_COPY_AND_ASSIGN(WorkerManager)
};

}  // namespace myframe
