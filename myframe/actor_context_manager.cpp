/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/actor_context_manager.h"

#include <assert.h>
#include <string.h>

#include <vector>

#include <glog/logging.h>

#include "myframe/msg.h"
#include "myframe/actor.h"
#include "myframe/actor_context.h"

namespace myframe {

ActorContextManager::ActorContextManager() : ctx_count_(0) {
  LOG(INFO) << "ActorContextManager create";
  pthread_rwlock_init(&rw_, NULL);
}

ActorContextManager::~ActorContextManager() {
  LOG(INFO) << "ActorContextManager deconstruct";
  pthread_rwlock_destroy(&rw_);
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
  pthread_rwlock_wrlock(&rw_);
  if (ctxs_.find(ctx->GetActor()->GetActorName()) != ctxs_.end()) {
    LOG(WARNING) << "reg the same actor name: "
                 << ctx->GetActor()->GetActorName();
    pthread_rwlock_unlock(&rw_);
    return false;
  }
  LOG(INFO) << "reg actor " << ctx->GetActor()->GetActorName();
  ctxs_[ctx->GetActor()->GetActorName()] = ctx;
  pthread_rwlock_unlock(&rw_);
  return true;
}

std::shared_ptr<ActorContext> ActorContextManager::GetContext(
    const std::string& actor_name) {
  pthread_rwlock_rdlock(&rw_);
  if (ctxs_.find(actor_name) == ctxs_.end()) {
    LOG(WARNING) << "not found " << actor_name;
    pthread_rwlock_unlock(&rw_);
    return nullptr;
  }
  auto ctx = ctxs_[actor_name];
  pthread_rwlock_unlock(&rw_);
  return ctx;
}

std::vector<std::string> ActorContextManager::GetAllActorAddr() {
  std::vector<std::string> res;
  pthread_rwlock_rdlock(&rw_);
  for (auto ctx : ctxs_) {
    res.push_back(ctx.first);
  }
  pthread_rwlock_unlock(&rw_);
  return res;
}

bool ActorContextManager::HasActor(const std::string& name) {
  bool res = false;
  pthread_rwlock_rdlock(&rw_);
  res = (ctxs_.find(name) != ctxs_.end());
  pthread_rwlock_unlock(&rw_);
  return res;
}

void ActorContextManager::PrintWaitQueue() {
  DLOG(INFO) << "cur wait queue actor:";
  auto it = wait_queue_.begin();
  while (it != wait_queue_.end()) {
    auto ctx = it->lock();
    if (ctx == nullptr) {
      LOG(ERROR) << "context is nullptr";
      continue;
    }
    DLOG(INFO) << "---> " << *ctx;
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
  for (int i = 0; i < in_runing_context.size(); ++i) {
    DLOG(INFO) << in_runing_context[i]->GetActor()->GetActorName()
               << " is runing, move to wait queue back";
    wait_queue_.push_back(in_runing_context[i]);
  }
  return ret;
}

void ActorContextManager::PushContext(std::shared_ptr<ActorContext> ctx) {
  if (ctx->IsInWaitQueue()) {
    DLOG(INFO) << *ctx << " already in wait queue, return";
    PrintWaitQueue();
    return;
  }
  ctx->SetWaitQueueFlag(true);
  wait_queue_.push_back(ctx);
  PrintWaitQueue();
}

}  // namespace myframe
