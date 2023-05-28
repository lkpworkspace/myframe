/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include "myframe/worker_context.h"

#include <functional>

#include <glog/logging.h>

#include "myframe/common.h"
#include "myframe/msg.h"
#include "myframe/worker.h"
#include "myframe/app.h"

namespace myframe {

WorkerContext::WorkerContext(
  std::shared_ptr<App> app,
  std::shared_ptr<Worker> worker)
  : runing_(false)
  , worker_(worker)
  , app_(app) {
}

WorkerContext::~WorkerContext() {
  LOG(INFO) << worker_->GetWorkerName() << " deconstruct";
}

int WorkerContext::GetFd() const {
  return cmd_channel_.GetMainFd();
}

EventType WorkerContext::GetType() {
  return worker_->GetType();
}

void WorkerContext::Start() {
  if (runing_.load() == false) {
    runing_.store(true);
    th_ = std::thread(
      std::bind(
        &WorkerContext::ListenThread,
        std::dynamic_pointer_cast<WorkerContext>(shared_from_this())));
  }
}

void WorkerContext::Stop() {
  runing_.store(false);
}

void WorkerContext::Join() {
  if (th_.joinable()) {
    th_.join();
  }
}

void WorkerContext::Initialize() {
  mailbox_.SetAddr(worker_->GetWorkerName());
  worker_->Init();
}

void WorkerContext::ListenThread(std::shared_ptr<WorkerContext> w) {
  if (w->worker_ == nullptr) {
    return;
  }
  w->Initialize();
  while (w->runing_.load()) {
    w->worker_->Run();
  }
  w->worker_->Exit();
  w->cmd_channel_.SendToMain(Cmd::kQuit);
}

std::size_t WorkerContext::CacheSize() const {
  return cache_.size();
}

std::list<std::shared_ptr<Msg>>* WorkerContext::GetCache() {
  return &cache_;
}

void WorkerContext::Cache(std::shared_ptr<Msg> msg) {
  cache_.emplace_back(msg);
}

void WorkerContext::Cache(std::list<std::shared_ptr<Msg>>* msg_list) {
  Common::ListAppend(&cache_, msg_list);
}

Mailbox* WorkerContext::GetMailbox() {
  return &mailbox_;
}

CmdChannel* WorkerContext::GetCmdChannel() {
  return &cmd_channel_;
}

std::shared_ptr<App> WorkerContext::GetApp() {
  return app_.lock();
}

std::ostream& operator<<(std::ostream& out, WorkerContext& ctx) {
  auto w = ctx.GetWorker<Worker>();
  out << w->GetWorkerName() << "." << ctx.GetPosixThreadId();
  return out;
}

}  // namespace myframe
