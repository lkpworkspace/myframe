/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/context.h"

#include <assert.h>

#include <sstream>

#include <glog/logging.h>

#include "myframe/actor.h"
#include "myframe/app.h"
#include "myframe/msg.h"

namespace myframe {

Context::Context(std::shared_ptr<App> app, std::shared_ptr<Actor> actor)
    : app_(app), actor_(actor), in_worker_(false), in_wait_que_(false) {
  mailbox_.SetAddr(actor_->GetActorName());
  LOG(INFO) << mailbox_.Addr() << " context create";
}

Context::~Context() {
  LOG(INFO) << mailbox_.Addr() << " context deconstruct";
}

std::shared_ptr<App> Context::GetApp() { return app_.lock(); }

int Context::Init(const char* param) {
  actor_->SetContext(shared_from_this());
  return actor_->Init(param);
}

Mailbox* Context::GetMailbox() {
  return &mailbox_;
}

void Context::Proc(const std::shared_ptr<const Msg>& msg) {
  actor_->Proc(msg);
}

std::ostream& operator<<(std::ostream& out, const Context& ctx) {
  out << ctx.actor_->GetActorName() << ", in worker: " << ctx.in_worker_
     << ", in wait queue: " << ctx.in_wait_que_;
  return out;
}

}  // namespace myframe
