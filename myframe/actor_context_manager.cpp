/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#include "myframe/actor_context_manager.h"

#include <vector>
#include <utility>

#include "myframe/log.h"
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

void ActorContextManager::DispatchMsg(
    std::shared_ptr<Msg> msg,
    const std::string& dst) {
  std::string actor_name = dst.empty()? msg->GetDst() : dst;
  auto ctx = GetContext(actor_name);
  if (nullptr == ctx) {
    LOG(ERROR) << "Unknown msg " << *msg;
    return;
  }
  auto mailbox = ctx->GetMailbox();
  mailbox->Recv(std::move(msg));
  PushContext(std::move(ctx));
}

bool ActorContextManager::RegContext(std::shared_ptr<ActorContext> ctx) {
  std::unique_lock<std::shared_mutex> lk(rw_);
  if (ctxs_.find(ctx->GetActor()->GetActorName()) != ctxs_.end()) {
    LOG(WARNING) << "reg the same actor name: "
                 << ctx->GetActor()->GetActorName();
    return false;
  }
  LOG(INFO) << "reg actor " << ctx->GetActor()->GetActorName();
  ctxs_[ctx->GetActor()->GetActorName()] = std::move(ctx);
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
  std::shared_lock<std::shared_mutex> lk(rw_);
  return ctxs_.find(name) != ctxs_.end();
}

void ActorContextManager::PrintWaitQueue() {
  if (!VLOG_IS_ON(1)) {
    return;
  }
  VLOG(1) << "cur wait queue actor:";
  auto it = wait_queue_.begin();
  while (it != wait_queue_.end()) {
    auto ctx = it->lock();
    if (ctx == nullptr) {
      LOG(ERROR) << "context is nullptr";
      continue;
    }
    VLOG(1) << "|--> " << *ctx;
    ++it;
  }
}

std::shared_ptr<ActorContext> ActorContextManager::GetContextWithMsg() {
  if (wait_queue_.empty()) {
    return nullptr;
  }

  std::list<std::weak_ptr<ActorContext>> in_runing_context;
  std::shared_ptr<ActorContext> ret = nullptr;
  while (!wait_queue_.empty()) {
    if (wait_queue_.front().expired()) {
      wait_queue_.pop_front();
      continue;
    }
    auto ctx = wait_queue_.front().lock();
    if (ctx->IsRuning()) {
      wait_queue_.pop_front();
      VLOG(1) << ctx->GetActor()->GetActorName()
              << " is runing, move to wait queue back";
      in_runing_context.push_back(std::move(ctx));
    } else {
      wait_queue_.pop_front();
      ctx->SetRuningFlag(true);
      ctx->SetWaitQueueFlag(false);
      ret.swap(ctx);
      break;
    }
  }
  if (!in_runing_context.empty()) {
    wait_queue_.splice(wait_queue_.end(), in_runing_context);
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
  wait_queue_.push_back(std::move(ctx));
  PrintWaitQueue();
}

}  // namespace myframe
