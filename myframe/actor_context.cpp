/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#include "myframe/actor_context.h"

#include <sstream>

#include "myframe/log.h"
#include "myframe/actor.h"
#include "myframe/app.h"
#include "myframe/msg.h"

namespace myframe {

ActorContext::ActorContext(
  std::shared_ptr<App> app,
  std::shared_ptr<Actor> actor)
    : in_worker_(false)
    , in_wait_que_(false)
    , actor_(actor)
    , app_(app) {
  actor_->SetContext(this);
  mailbox_.SetAddr(actor_->GetActorName());
  int pending_queue_size = app->GetDefaultPendingQueueSize();
  int run_queue_size = app->GetDefaultRunQueueSize();
  auto cfg = actor_->GetConfig();
  if (cfg->isMember("pending_queue_size")) {
    pending_queue_size = cfg->get("pending_queue_size", -1).asInt();
  }
  if (cfg->isMember("run_queue_size")) {
    run_queue_size = cfg->get("run_queue_size", -1).asInt();
  }
  mailbox_.SetPendingQueueSize(pending_queue_size);
  mailbox_.SetRunQueueSize(run_queue_size);
  LOG(INFO) << mailbox_.Addr() << " context create";
}

ActorContext::~ActorContext() {
  LOG(INFO) << mailbox_.Addr() << " context deconstruct";
}

std::shared_ptr<App> ActorContext::GetApp() { return app_.lock(); }

int ActorContext::Init(const char* param) {
  return actor_->Init(param);
}

Mailbox* ActorContext::GetMailbox() {
  return &mailbox_;
}

void ActorContext::Proc(const std::shared_ptr<const Msg>& msg) {
  actor_->Proc(msg);
}

std::ostream& operator<<(std::ostream& out, const ActorContext& ctx) {
  out << ctx.actor_->GetActorName() << ", in worker: " << ctx.in_worker_
     << ", in wait queue: " << ctx.in_wait_que_;
  return out;
}

}  // namespace myframe
