/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/worker_context.h"

#include <functional>
#include <utility>

#include "myframe/log.h"
#include "myframe/msg.h"
#include "myframe/worker.h"
#include "myframe/app.h"
#include "myframe/common.h"

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

bool WorkerContext::SetThreadAffinity(int cpu_core) {
  if (runing_.load()) {
    if (0 == Common::SetThreadAffinity(&th_, cpu_core)) {
      return true;
    }
    LOG(WARNING) << GetName() << " bind cpu " << cpu_core << " failed";
  } else {
    LOG(WARNING) << GetName() << " not runing, skip SetThreadAffinity";
  }
  return false;
}

void WorkerContext::Initialize() {
  mailbox_.SetAddr(worker_->GetWorkerName());
  std::string th_name = mailbox_.Addr();
  th_name = th_name.size() >= 16 ? th_name.substr(0, 15) : th_name;
  if (Common::SetSelfThreadName(th_name)) {
    LOG(WARNING) << "set thread name " << th_name << " failed";
  }
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
