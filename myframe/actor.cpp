/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/actor.h"

#include "myframe/app.h"
#include "myframe/context.h"
#include "myframe/worker_timer.h"

namespace myframe {

Actor::Actor() {}

Actor::~Actor() {}

void Actor::SetModName(const std::string& name) {
  if (mod_name_ == "class") {
    is_from_lib_ = false;
  } else {
    is_from_lib_ = true;
  }
  mod_name_ = name;
}

int Actor::Send(const std::string& dst, std::shared_ptr<Msg> msg) {
  auto ctx = ctx_.lock();
  if (ctx == nullptr) {
    return -1;
  }
  msg->SetSrc(GetActorName());
  msg->SetDst(dst);
  return ctx->SendMsg(msg);
}

int Actor::Send(const std::string& dst, std::any data) {
  auto msg = std::make_shared<Msg>(data);
  return Send(dst, msg);
}

const std::string Actor::GetActorName() const {
  return "actor." + actor_name_ + "." + instance_name_;
}

int Actor::Timeout(const std::string& timer_name, int expired) {
  auto ctx = ctx_.lock();
  if (ctx == nullptr) {
    return -1;
  }
  auto app = ctx->GetApp();
  if (app == nullptr) {
    return -1;
  }
  auto timer_worker = app->GetTimerWorker();
  if (timer_worker == nullptr) {
    return -1;
  }
  return timer_worker->SetTimeout(GetActorName(), timer_name, expired);
}

void Actor::SetContext(std::shared_ptr<Context> c) { ctx_ = c; }

}  // namespace myframe
