/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#include "myframe/actor_context_manager.h"

#include <vector>

#include <glog/logging.h>

#include "myframe/msg.h"
#include "myframe/actor.h"
#include "myframe/actor_context.h"

namespace myframe {

ActorContextManager::ActorContextManager() : ctx_count_(0) {
  LOG(INFO) << "ActorContextManager create";
}

ActorContextManager::~ActorContextManager() {
  LOG(INFO) << "ActorContextManager deconstruct";
}

void ActorContextManager::DispatchMsg(std::shared_ptr<Msg> msg) {
  auto ctx = GetContext(msg->GetDst());
  if (nullptr == ctx) {
    LOG(ERROR) << "Unknown msg " << *msg;
    return;
  }
  auto mailbox = ctx->GetMailbox();
  mailbox->Recv(msg);
  PushContext(ctx);
}

bool ActorContextManager::RegContext(std::shared_ptr<ActorContext> ctx) {
  std::unique_lock<std::shared_mutex> lk(rw_);
  if (ctxs_.find(ctx->GetActor()->GetActorName()) != ctxs_.end()) {
    LOG(WARNING) << "reg the same actor name: "
                 << ctx->GetActor()->GetActorName();
    return false;
  }
  LOG(INFO) << "reg actor " << ctx->GetActor()->GetActorName();
  ctxs_[ctx->GetActor()->GetActorName()] = ctx;
  return true;
}

std::shared_ptr<ActorContext> ActorContextManager::GetContext(
    const std::string& actor_name) {
  std::shared_lock<std::shared_mutex> lk(rw_);
  if (ctxs_.find(actor_name) == ctxs_.end()) {
    LOG(WARNING) << "not found " << actor_name;
    return nullptr;
  }
  auto ctx = ctxs_[actor_name];
  return ctx;
}

std::vector<std::string> ActorContextManager::GetAllActorAddr() {
  std::vector<std::string> res;
  std::shared_lock<std::shared_mutex> lk(rw_);
  for (auto ctx : ctxs_) {
    res.push_back(ctx.first);
  }
  return res;
}

bool ActorContextManager::HasActor(const std::string& name) {
  bool res = false;
  std::shared_lock<std::shared_mutex> lk(rw_);
  res = (ctxs_.find(name) != ctxs_.end());
  return res;
}

void ActorContextManager::PrintWaitQueue() {
  VLOG(1) << "cur wait queue actor:";
  auto it = wait_queue_.begin();
  while (it != wait_queue_.end()) {
    auto ctx = it->lock();
    if (ctx == nullptr) {
      LOG(ERROR) << "context is nullptr";
      continue;
    }
    VLOG(1) << "---> " << *ctx;
    ++it;
  }
}

std::shared_ptr<ActorContext> ActorContextManager::GetContextWithMsg() {
  if (wait_queue_.empty()) {
    return nullptr;
  }

  std::vector<std::shared_ptr<ActorContext>> in_runing_context;
  std::shared_ptr<ActorContext> ret = nullptr;
  while (!wait_queue_.empty()) {
    if (wait_queue_.front().expired()) {
      wait_queue_.pop_front();
      continue;
    }
    auto ctx = wait_queue_.front().lock();
    if (ctx->IsRuning()) {
      wait_queue_.pop_front();
      in_runing_context.push_back(ctx);
    } else {
      wait_queue_.pop_front();

      ctx->SetRuningFlag(true);
      ctx->SetWaitQueueFlag(false);
      ret = ctx;
      break;
    }
  }
  for (std::size_t i = 0; i < in_runing_context.size(); ++i) {
    VLOG(1) << in_runing_context[i]->GetActor()->GetActorName()
               << " is runing, move to wait queue back";
    wait_queue_.push_back(in_runing_context[i]);
  }
  return ret;
}

void ActorContextManager::PushContext(std::shared_ptr<ActorContext> ctx) {
  if (ctx->IsInWaitQueue()) {
    VLOG(1) << *ctx << " already in wait queue, return";
    PrintWaitQueue();
    return;
  }
  ctx->SetWaitQueueFlag(true);
  wait_queue_.push_back(ctx);
  PrintWaitQueue();
}

}  // namespace myframe
