/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#include "myframe/actor.h"
#include <utility>

#include "myframe/log.h"
#include "myframe/app.h"
#include "myframe/actor_context.h"
#include "myframe/worker_timer.h"
#include "myframe/mailbox.h"

namespace myframe {

Actor::~Actor() {}

void Actor::SetModName(const std::string& name) {
  mod_name_ = name;
}

const std::string& Actor::GetModName() const {
  return mod_name_;
}

Mailbox* Actor::GetMailbox() {
  if (ctx_ == nullptr) {
    return nullptr;
  }
  return ctx_->GetMailbox();
}

const std::string& Actor::GetTypeName() const { return class_name_; }

const std::string& Actor::GetInstName() const { return instance_name_; }

const std::string Actor::GetActorName() const {
  return "actor." + class_name_ + "." + instance_name_;
}

void Actor::SetTypeName(const std::string& name) { class_name_ = name; }

void Actor::SetInstName(const std::string& name) { instance_name_ = name; }

int Actor::Timeout(const std::string& timer_name, int expired) {
  if (ctx_ == nullptr) {
    LOG(ERROR) << "actor context is nullptr";
    return -1;
  }
  auto app = ctx_->GetApp();
  if (app == nullptr) {
    LOG(ERROR) << "app is nullptr";
    return -1;
  }
  auto timer_worker = app->GetTimerWorker();
  if (timer_worker == nullptr) {
    LOG(ERROR) << "timer worker is nullptr";
    return -1;
  }
  return timer_worker->SetTimeout(GetActorName(), timer_name, expired);
}

bool Actor::Subscribe(
  const std::string& addr,
  const std::string& msg_name,
  const Msg::TransMode mode) {
  if (ctx_ == nullptr) {
    return false;
  }
  if (addr == GetActorName()) {
    return false;
  }
  auto msg = std::make_shared<Msg>();
  msg->SetType(MYFRAME_MSG_TYPE_SUB);
  msg->SetName(msg_name);
  msg->SetTransMode(mode);
  auto mailbox = ctx_->GetMailbox();
  mailbox->Send(addr, std::move(msg));
  return true;
}

bool Actor::Publish(std::shared_ptr<Msg> msg) {
  if (ctx_ == nullptr) {
    return false;
  }
  msg->SetSrc(GetActorName());
  msg->SetDst("");
  msg->SetType(MYFRAME_MSG_TYPE_PUB);
  auto mailbox = ctx_->GetMailbox();
  mailbox->Send(std::move(msg));
  return true;
}

void Actor::SetContext(ActorContext* c) { ctx_ = c; }

const Json::Value* Actor::GetConfig() const {
  return ctx_->GetConfig();
}

std::shared_ptr<App> Actor::GetApp() {
  if (ctx_ == nullptr) {
    return nullptr;
  }
  return ctx_->GetApp();
}

}  // namespace myframe
