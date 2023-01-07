/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/context.h"

#include <assert.h>

#include <sstream>

#include "myframe/actor.h"
#include "myframe/app.h"
#include "myframe/log.h"
#include "myframe/msg.h"

namespace myframe {

Context::Context(std::shared_ptr<App> app, std::shared_ptr<Actor> mod)
    : app_(app), actor_(mod), in_worker_(false), in_wait_que_(false) {
  LOG(INFO) << actor_->GetActorName() << " context create";
}

Context::~Context() {
  LOG(INFO) << actor_->GetActorName() << " context deconstruct";
}

std::shared_ptr<App> Context::GetApp() { return app_.lock(); }

int Context::SendMsg(std::shared_ptr<Msg> msg) {
  if (nullptr == msg) {
    return -1;
  }
  send_.emplace_back(msg);
  return 0;
}

int Context::Init(const char* param) {
  actor_->SetContext(shared_from_this());
  return actor_->Init(param);
}

void Context::Proc(const std::shared_ptr<const Msg>& msg) {
  actor_->Proc(msg);
}

std::string Context::Print() {
  std::stringstream ss;
  ss << "context " << actor_->GetActorName() << ", in worker: " << in_worker_
     << ", in wait queue: " << in_wait_que_;
  return ss.str();
}

}  // namespace myframe
