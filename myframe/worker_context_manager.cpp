/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/worker_context_manager.h"

#include <glog/logging.h>

#include "myframe/common.h"
#include "myframe/msg.h"
#include "myframe/worker.h"
#include "myframe/worker_context.h"

namespace myframe {

WorkerContextManager::WorkerContextManager() {
  LOG(INFO) << "WorkerContextManager create";
  pthread_rwlock_init(&rw_, NULL);
}

WorkerContextManager::~WorkerContextManager() {
  LOG(INFO) << "WorkerContextManager deconstruct";
  pthread_rwlock_destroy(&rw_);
}

bool WorkerContextManager::Init(int warning_msg_size) {
  warning_msg_size_.store(warning_msg_size);
  return true;
}

int WorkerContextManager::WorkerSize() { return cur_worker_count_; }

std::shared_ptr<WorkerContext> WorkerContextManager::Get(int handle) {
  pthread_rwlock_rdlock(&rw_);
  if (worker_ctxs_.find(handle) == worker_ctxs_.end()) {
    DLOG(WARNING) << "can't find worker, handle " << handle;
    pthread_rwlock_unlock(&rw_);
    return nullptr;
  }
  auto ret = worker_ctxs_[handle];
  pthread_rwlock_unlock(&rw_);
  return ret;
}

std::shared_ptr<WorkerContext> WorkerContextManager::Get(
  const std::string& name) {
  pthread_rwlock_rdlock(&rw_);
  if (name_handle_map_.find(name) == name_handle_map_.end()) {
    LOG(ERROR) << "can't find worker, name " << name;
    pthread_rwlock_unlock(&rw_);
    return nullptr;
  }
  auto handle = name_handle_map_[name];
  auto ret = worker_ctxs_[handle];
  pthread_rwlock_unlock(&rw_);
  return ret;
}

bool WorkerContextManager::Add(std::shared_ptr<WorkerContext> worker_ctx) {
  auto worker = worker_ctx->GetWorker<Worker>();
  int handle = worker_ctx->GetFd();
  pthread_rwlock_wrlock(&rw_);
  if (worker_ctxs_.find(handle) != worker_ctxs_.end()) {
    LOG(ERROR) << *worker_ctx << " reg handle " << handle
               << " has exist";
    pthread_rwlock_unlock(&rw_);
    return false;
  }
  worker_ctxs_[handle] = worker_ctx;
  name_handle_map_[worker->GetWorkerName()] = handle;
  ++cur_worker_count_;
  pthread_rwlock_unlock(&rw_);
  return true;
}

void WorkerContextManager::Del(std::shared_ptr<WorkerContext> worker_ctx) {
  auto worker = worker_ctx->GetWorker<Worker>();
  int handle = worker_ctx->GetFd();
  pthread_rwlock_wrlock(&rw_);
  if (worker_ctxs_.find(handle) == worker_ctxs_.end()) {
    pthread_rwlock_unlock(&rw_);
    return;
  }
  stoped_workers_ctx_.push_back(worker_ctx);
  worker_ctxs_.erase(worker_ctxs_.find(handle));
  name_handle_map_.erase(worker->GetWorkerName());
  --cur_worker_count_;
  pthread_rwlock_unlock(&rw_);
}

int WorkerContextManager::IdleWorkerSize() {
  int sz = 0;
  pthread_rwlock_rdlock(&rw_);
  sz = idle_workers_ctx_.size();
  pthread_rwlock_unlock(&rw_);
  return sz;
}

std::shared_ptr<WorkerContext> WorkerContextManager::FrontIdleWorker() {
  std::shared_ptr<WorkerContext> w = nullptr;
  pthread_rwlock_rdlock(&rw_);
  if (idle_workers_ctx_.empty()) {
    pthread_rwlock_unlock(&rw_);
    return nullptr;
  }
  w = idle_workers_ctx_.front().lock();
  pthread_rwlock_unlock(&rw_);
  return w;
}

void WorkerContextManager::PopFrontIdleWorker() {
  pthread_rwlock_wrlock(&rw_);
  if (idle_workers_ctx_.empty()) {
    pthread_rwlock_unlock(&rw_);
    return;
  }
  idle_workers_ctx_.pop_front();
  pthread_rwlock_unlock(&rw_);
}

void WorkerContextManager::PushBackIdleWorker(
  std::shared_ptr<WorkerContext> worker) {
  pthread_rwlock_wrlock(&rw_);
  idle_workers_ctx_.emplace_back(worker);
  pthread_rwlock_unlock(&rw_);
}

bool WorkerContextManager::HasWorker(const std::string& name) {
  bool res = false;
  pthread_rwlock_rdlock(&rw_);
  res = (name_handle_map_.find(name) != name_handle_map_.end());
  pthread_rwlock_unlock(&rw_);
  return res;
}

std::vector<std::string> WorkerContextManager::GetAllUserWorkerAddr() {
  std::vector<std::string> res;
  pthread_rwlock_rdlock(&rw_);
  for (auto p : worker_ctxs_) {
    if (p.second->GetType() == EventType::kWorkerUser
        && p.second->GetWorker<Worker>()->GetTypeName() != "node") {
      res.push_back(p.second->GetWorker<Worker>()->GetWorkerName());
    }
  }
  pthread_rwlock_unlock(&rw_);
  return res;
}

void WorkerContextManager::StopAllWorker() {
  pthread_rwlock_rdlock(&rw_);
  for (auto p : worker_ctxs_) {
    // 目前仅支持使用channel通信的worker停止退出
    // 不使用的可以调用Stop函数退出(目前暂无需求)
    //   p.second->Stop();
    p.second->GetCmdChannel()->SendToOwner(Cmd::kQuit);
  }
  pthread_rwlock_unlock(&rw_);
}

void WorkerContextManager::WaitAllWorkerQuit() {
  // FIXME(likepeng): 只支持退出时释放worker资源
  // 运行时释放worker资源有可能导致主线程阻塞，影响其它组件调度
  pthread_rwlock_rdlock(&rw_);
  for (auto p : stoped_workers_ctx_) {
    p->Join();
  }
  pthread_rwlock_unlock(&rw_);
}

void WorkerContextManager::PushWaitWorker(
  std::shared_ptr<WorkerContext> worker) {
  worker->SetCtrlOwnerFlag(WorkerCtrlOwner::kMain);
}

void WorkerContextManager::WeakupWorker() {
  pthread_rwlock_wrlock(&rw_);
  for (auto it = weakup_workers_ctx_.begin();
    it != weakup_workers_ctx_.end();) {
    auto worker_ctx = it->lock();
    if (worker_ctx == nullptr) {
      it = weakup_workers_ctx_.erase(it);
      continue;
    }
    if (worker_ctx->GetOwner() == WorkerCtrlOwner::kWorker) {
      ++it;
      continue;
    }
    worker_ctx->GetMailbox()->Recv(worker_ctx->GetCache());
    it = weakup_workers_ctx_.erase(it);
    worker_ctx->SetCtrlOwnerFlag(WorkerCtrlOwner::kWorker);
    worker_ctx->SetWaitMsgQueueFlag(false);
    DLOG(INFO) << "notify " << *worker_ctx << " process msg";
    worker_ctx->GetCmdChannel()->SendToOwner(Cmd::kRunWithMsg);
  }
  pthread_rwlock_unlock(&rw_);
}

void WorkerContextManager::DispatchWorkerMsg(std::shared_ptr<Msg> msg) {
  std::string worker_name = msg->GetDst();
  if (name_handle_map_.find(worker_name) == name_handle_map_.end()) {
    LOG(ERROR) << "can't find worker " << worker_name << ", drop msg: from "
               << msg->GetSrc() << " to " << msg->GetDst();
    return;
  }
  auto worker_ctx = Get(worker_name);
  auto worker = worker_ctx->GetWorker<Worker>();
  auto worker_type = worker->GetType();
  if (worker_type == EventType::kWorkerTimer ||
      worker_type == EventType::kWorkerCommon) {
    LOG(WARNING) << worker_name << " unsupport recv msg, drop it";
    return;
  }
  worker_ctx->Cache(msg);
  LOG_IF(WARNING,
    worker_ctx->CacheSize() > warning_msg_size_.load())
      << *worker_ctx << " has " << worker_ctx->CacheSize()
      << " msg not process!!!";
  if (worker_ctx->IsInWaitMsgQueue()) {
    DLOG(INFO) << *worker_ctx << " already in wait queue, return";
    return;
  }
  worker_ctx->SetWaitMsgQueueFlag(true);
  pthread_rwlock_wrlock(&rw_);
  weakup_workers_ctx_.emplace_back(worker_ctx);
  pthread_rwlock_unlock(&rw_);
}

}  // namespace myframe
