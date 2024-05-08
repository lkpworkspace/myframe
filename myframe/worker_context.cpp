/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/worker_context.h"

#include <functional>

#include "myframe/log.h"
#include "myframe/common.h"
#include "myframe/msg.h"
#include "myframe/worker.h"
#include "myframe/app.h"

namespace myframe {

WorkerContext::WorkerContext(
  std::shared_ptr<App> app,
  std::shared_ptr<Worker> worker,
  std::shared_ptr<Poller> poller)
  : runing_(false)
  , worker_(worker)
  , app_(app) {
  worker_->SetContext(this);
  cmd_channel_ = CmdChannel::Create(poller);
}

WorkerContext::~WorkerContext() {
  LOG(INFO) << worker_->GetWorkerName() << " deconstruct";
}

ev_handle_t WorkerContext::GetHandle() const {
  return cmd_channel_->GetMainHandle();
}

Event::Type WorkerContext::GetType() const {
  return worker_->GetType();
}

std::string WorkerContext::GetName() const {
  return worker_->GetWorkerName();
}

void WorkerContext::Start() {
  if (runing_.load() == false) {
    runing_.store(true);
    th_ = std::thread(std::bind(&WorkerContext::ListenThread, this));
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

void WorkerContext::ListenThread() {
  if (worker_ == nullptr) {
    return;
  }
  Initialize();
  while (runing_.load()) {
    worker_->Run();
  }
  worker_->Exit();
  cmd_channel_->SendToMain(CmdChannel::Cmd::kQuit);
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
  return cmd_channel_.get();
}

std::shared_ptr<App> WorkerContext::GetApp() {
  return app_.lock();
}

std::ostream& operator<<(std::ostream& out, WorkerContext& ctx) {
  auto w = ctx.GetWorker<Worker>();
  out << w->GetWorkerName() << "." << ctx.GetThreadId();
  return out;
}

}  // namespace myframe
