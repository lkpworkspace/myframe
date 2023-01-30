/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/actor.h"

#include "myframe/app.h"
#include "myframe/actor_context.h"
#include "myframe/worker_timer.h"

namespace myframe {

Actor::~Actor() {}

void Actor::SetModName(const std::string& name) {
  if (mod_name_ == "class") {
    is_from_lib_ = false;
  } else {
    is_from_lib_ = true;
  }
  mod_name_ = name;
}

const std::string& Actor::GetModName() const {
  return mod_name_;
}

bool Actor::IsFromLib() const { return is_from_lib_; }

Mailbox* Actor::GetMailbox() {
  auto ctx = ctx_.lock();
  if (ctx == nullptr) {
    return nullptr;
  }
  return ctx->GetMailbox();
}

const std::string& Actor::GetTypeName() const { return actor_name_; }

const std::string& Actor::GetInstName() const { return instance_name_; }

const std::string Actor::GetActorName() const {
  return "actor." + actor_name_ + "." + instance_name_;
}

void Actor::SetTypeName(const std::string& name) { actor_name_ = name; }

void Actor::SetInstName(const std::string& name) { instance_name_ = name; }

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

void Actor::SetContext(std::shared_ptr<ActorContext> c) { ctx_ = c; }

const Json::Value* Actor::GetConfig() const {
  return &config_;
}

void Actor::SetConfig(const Json::Value& conf) {
  config_ = conf;
}

std::shared_ptr<App> Actor::GetApp() {
  auto ctx = ctx_.lock();
  if (ctx == nullptr) {
    return nullptr;
  }
  return ctx->GetApp();
}

}  // namespace myframe
