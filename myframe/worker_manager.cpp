/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/worker_manager.h"

#include <glog/logging.h>

#include "myframe/flags.h"
#include "myframe/common.h"
#include "myframe/worker.h"

namespace myframe {

WorkerManager::WorkerManager() {
  LOG(INFO) << "WorkerManager create";
  pthread_rwlock_init(&rw_, NULL);
}

WorkerManager::~WorkerManager() {
  LOG(INFO) << "WorkerManager deconstruct";
  pthread_rwlock_destroy(&rw_);
}

int WorkerManager::WorkerSize() { return cur_worker_count_; }

std::shared_ptr<Worker> WorkerManager::Get(int handle) {
  pthread_rwlock_rdlock(&rw_);
  if (workers_.find(handle) == workers_.end()) {
    DLOG(WARNING) << "can't find worker, handle " << handle;
    pthread_rwlock_unlock(&rw_);
    return nullptr;
  }
  auto ret = workers_[handle];
  pthread_rwlock_unlock(&rw_);
  return ret;
}

std::shared_ptr<Worker> WorkerManager::Get(const std::string& name) {
  pthread_rwlock_rdlock(&rw_);
  if (name_handle_map_.find(name) == name_handle_map_.end()) {
    LOG(ERROR) << "can't find worker, name " << name;
    pthread_rwlock_unlock(&rw_);
    return nullptr;
  }
  auto handle = name_handle_map_[name];
  auto ret = workers_[handle];
  pthread_rwlock_unlock(&rw_);
  return ret;
}

bool WorkerManager::Add(std::shared_ptr<Worker> worker) {
  int handle = worker->GetFd();
  pthread_rwlock_wrlock(&rw_);
  if (workers_.find(handle) != workers_.end()) {
    LOG(ERROR) << worker->GetWorkerName() << " reg handle " << handle
               << " has exist";
    pthread_rwlock_unlock(&rw_);
    return false;
  }
  workers_[handle] = worker;
  name_handle_map_[worker->GetWorkerName()] = handle;
  auto ev_type = worker->GetType();
  if (ev_type == EventType::WORKER_COMMON ||
      ev_type == EventType::WORKER_USER) {
    ++cur_worker_count_;
  }
  pthread_rwlock_unlock(&rw_);
  return true;
}

void WorkerManager::Del(std::shared_ptr<Worker> worker) {
  int handle = worker->GetFd();
  pthread_rwlock_wrlock(&rw_);
  if (workers_.find(handle) == workers_.end()) {
    pthread_rwlock_unlock(&rw_);
    return;
  }
  workers_.erase(workers_.find(handle));
  name_handle_map_.erase(worker->GetWorkerName());
  auto ev_type = worker->GetType();
  if (ev_type == EventType::WORKER_COMMON ||
      ev_type == EventType::WORKER_USER) {
    --cur_worker_count_;
  }
  pthread_rwlock_unlock(&rw_);
}

int WorkerManager::IdleWorkerSize() {
  int sz = 0;
  pthread_rwlock_rdlock(&rw_);
  sz = idle_workers_.size();
  pthread_rwlock_unlock(&rw_);
  return sz;
}

std::shared_ptr<Worker> WorkerManager::FrontIdleWorker() {
  std::shared_ptr<Worker> w = nullptr;
  pthread_rwlock_rdlock(&rw_);
  if (idle_workers_.empty()) {
    pthread_rwlock_unlock(&rw_);
    return nullptr;
  }
  w = idle_workers_.front().lock();
  pthread_rwlock_unlock(&rw_);
  return w;
}

void WorkerManager::PopFrontIdleWorker() {
  pthread_rwlock_wrlock(&rw_);
  if (idle_workers_.empty()) {
    pthread_rwlock_unlock(&rw_);
    return;
  }
  idle_workers_.pop_front();
  pthread_rwlock_unlock(&rw_);
}

void WorkerManager::PushBackIdleWorker(std::shared_ptr<Worker> worker) {
  pthread_rwlock_wrlock(&rw_);
  idle_workers_.emplace_back(worker);
  pthread_rwlock_unlock(&rw_);
}

void WorkerManager::PushWaitWorker(std::shared_ptr<Worker> worker) {
  worker->SetCtrlOwnerFlag(WorkerCtrlOwner::MAIN);
}

void WorkerManager::WeakupWorker() {
  pthread_rwlock_wrlock(&rw_);
  for (auto it = weakup_workers_.begin(); it != weakup_workers_.end();) {
    if (it->expired()) {
      it = weakup_workers_.erase(it);
      continue;
    }
    auto worker = it->lock();
    if (worker->GetOwner() == WorkerCtrlOwner::WORKER) {
      ++it;
      continue;
    }
    worker->GetMailbox()->Recv(worker->GetCache());
    it = weakup_workers_.erase(it);
    worker->SetCtrlOwnerFlag(WorkerCtrlOwner::WORKER);
    worker->SetWaitMsgQueueFlag(false);
    DLOG(INFO) << "notify " << worker->GetWorkerName() << " process msg";
    worker->SendCmdToWorker(WorkerCmd::RUN_WITH_MSG);
  }
  pthread_rwlock_unlock(&rw_);
}

void WorkerManager::DispatchWorkerMsg(std::shared_ptr<Msg> msg) {
  std::string worker_name = msg->GetDst();
  if (name_handle_map_.find(worker_name) == name_handle_map_.end()) {
    LOG(ERROR) << "can't find worker " << worker_name << ", drop msg: from "
               << msg->GetSrc() << " to " << msg->GetDst();
    return;
  }
  auto worker = Get(worker_name);
  auto worker_type = worker->GetType();
  if (worker_type == EventType::WORKER_TIMER ||
      worker_type == EventType::WORKER_COMMON) {
    LOG(WARNING) << worker_name << " unsupport recv msg, drop it";
    return;
  }
  worker->Cache(msg);
  LOG_IF(WARNING,
    worker->CacheSize() > myframe::FLAGS_myframe_dispatch_or_process_msg_max)
      << worker->GetWorkerName() << " has " << worker->CacheSize()
      << " msg not process!!!";
  if (worker->IsInWaitMsgQueue()) {
    DLOG(INFO) << worker->GetWorkerName() << " already in wait queue, return";
    return;
  }
  worker->SetWaitMsgQueueFlag(true);
  pthread_rwlock_wrlock(&rw_);
  weakup_workers_.emplace_back(worker);
  pthread_rwlock_unlock(&rw_);
}

}  // namespace myframe
