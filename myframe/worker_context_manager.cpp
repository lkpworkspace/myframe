/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#include "myframe/worker_context_manager.h"

#include <utility>

#include "myframe/log.h"
#include "myframe/msg.h"
#include "myframe/worker.h"
#include "myframe/worker_context.h"
#include "myframe/event_manager.h"

namespace myframe {

WorkerContextManager::WorkerContextManager(
    std::shared_ptr<EventManager> ev_mgr)
    : ev_mgr_(ev_mgr) {
  LOG(INFO) << "WorkerContextManager create";
}

WorkerContextManager::~WorkerContextManager() {
  LOG(INFO) << "WorkerContextManager deconstruct";
}

bool WorkerContextManager::Init(int warning_msg_size) {
  warning_msg_size_.store(warning_msg_size);
  return true;
}

int WorkerContextManager::WorkerSize() { return cur_worker_count_.load(); }

bool WorkerContextManager::Add(std::shared_ptr<WorkerContext> worker_ctx) {
  if (!ev_mgr_->Add(worker_ctx)) {
    return false;
  }
  cur_worker_count_.fetch_add(1);
  return true;
}

void WorkerContextManager::Del(std::shared_ptr<WorkerContext> worker_ctx) {
  if (!ev_mgr_->Del(worker_ctx)) {
    return;
  }
  std::unique_lock<std::shared_mutex> lk(rw_);
  stoped_workers_ctx_.push_back(worker_ctx);
  cur_worker_count_.fetch_sub(1);
}

int WorkerContextManager::IdleWorkerSize() {
  int sz = 0;
  std::shared_lock<std::shared_mutex> lk(rw_);
  sz = idle_workers_ctx_.size();
  return sz;
}

std::shared_ptr<WorkerContext> WorkerContextManager::FrontIdleWorker() {
  std::shared_ptr<WorkerContext> w = nullptr;
  std::shared_lock<std::shared_mutex> lk(rw_);
  if (idle_workers_ctx_.empty()) {
    return nullptr;
  }
  w = idle_workers_ctx_.front().lock();
  return w;
}

void WorkerContextManager::PopFrontIdleWorker() {
  std::unique_lock<std::shared_mutex> lk(rw_);
  if (idle_workers_ctx_.empty()) {
    return;
  }
  idle_workers_ctx_.pop_front();
}

void WorkerContextManager::PushBackIdleWorker(
  std::shared_ptr<WorkerContext> worker) {
  std::unique_lock<std::shared_mutex> lk(rw_);
  idle_workers_ctx_.push_back(std::move(worker));
}

std::vector<std::string> WorkerContextManager::GetAllUserWorkerAddr() {
  std::vector<std::string> res;
  std::shared_lock<std::shared_mutex> lk(rw_);
  auto worker_ctx_list = ev_mgr_->Get({Event::Type::kWorkerUser});
  for (auto p : worker_ctx_list) {
    auto worker_ctx = std::dynamic_pointer_cast<WorkerContext>(p);
    auto worker = worker_ctx->GetWorker<Worker>();
    // 主要服务与node类型的worker
    // 所以返回的地址不包含node
    if (worker->GetTypeName() != "node") {
      res.push_back(worker->GetWorkerName());
    }
  }
  return res;
}

void WorkerContextManager::StopAllWorker() {
  auto worker_ctx_list = ev_mgr_->Get({
    Event::Type::kWorkerUser,
    Event::Type::kWorkerCommon,
    Event::Type::kWorkerTimer});
  for (auto p : worker_ctx_list) {
    auto worker_ctx = std::dynamic_pointer_cast<WorkerContext>(p);
    // 目前仅支持使用channel通信的worker停止退出
    // 不使用的可以调用Stop函数退出(目前暂无需求)
    //   worker_ctx->Stop();
    worker_ctx->GetCmdChannel()->SendToOwner(CmdChannel::Cmd::kQuit);
  }
}

void WorkerContextManager::WaitAllWorkerQuit() {
  // FIXME(likepeng): 只支持退出时释放worker资源
  // 运行时释放worker资源有可能导致主线程阻塞，影响其它组件调度
  std::shared_lock<std::shared_mutex> lk(rw_);
  for (auto p : stoped_workers_ctx_) {
    p->Join();
  }
}

void WorkerContextManager::PushWaitWorker(
  std::shared_ptr<WorkerContext> worker) {
  worker->SetCtrlOwnerFlag(WorkerContext::CtrlOwner::kMain);
}

void WorkerContextManager::WeakupWorker() {
  std::unique_lock<std::shared_mutex> lk(rw_);
  for (auto it = weakup_workers_ctx_.begin();
    it != weakup_workers_ctx_.end();) {
    auto worker_ctx = it->lock();
    if (worker_ctx == nullptr) {
      it = weakup_workers_ctx_.erase(it);
      continue;
    }
    if (worker_ctx->GetOwner() == WorkerContext::CtrlOwner::kWorker) {
      ++it;
      continue;
    }
    worker_ctx->GetMailbox()->Recv(worker_ctx->GetCache());
    it = weakup_workers_ctx_.erase(it);
    worker_ctx->SetCtrlOwnerFlag(WorkerContext::CtrlOwner::kWorker);
    worker_ctx->SetWaitMsgQueueFlag(false);
    VLOG(1) << "notify " << *worker_ctx << " process msg";
    worker_ctx->GetCmdChannel()->SendToOwner(CmdChannel::Cmd::kRunWithMsg);
  }
}

void WorkerContextManager::DispatchWorkerMsg(
    std::shared_ptr<Msg> msg,
    const std::string& dst) {
  std::string worker_name = dst.empty() ? msg->GetDst() : dst;
  if (!ev_mgr_->Has(worker_name)) {
    LOG(ERROR) << "can't find worker " << worker_name << ", drop msg: from "
               << msg->GetSrc() << " to " << msg->GetDst();
    return;
  }
  auto worker_ctx = ev_mgr_->Get<WorkerContext>(worker_name);
  auto worker = worker_ctx->GetWorker<Worker>();
  auto worker_type = worker->GetType();
  if (worker_type == Event::Type::kWorkerTimer ||
      worker_type == Event::Type::kWorkerCommon) {
    LOG(WARNING) << worker_name << " unsupport recv msg, drop it";
    return;
  }
  worker_ctx->Cache(std::move(msg));
  LOG_IF(WARNING,
    worker_ctx->CacheSize() > warning_msg_size_.load())
      << *worker_ctx << " has " << worker_ctx->CacheSize()
      << " msg not process!!!";
  if (worker_ctx->IsInWaitMsgQueue()) {
    VLOG(1) << *worker_ctx << " already in wait queue, return";
    return;
  }
  worker_ctx->SetWaitMsgQueueFlag(true);
  std::unique_lock<std::shared_mutex> lk(rw_);
  weakup_workers_ctx_.emplace_back(worker_ctx);
}

}  // namespace myframe
