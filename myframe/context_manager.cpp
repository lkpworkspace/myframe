/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/context_manager.h"

#include <assert.h>
#include <string.h>

#include <vector>

#include "myframe/actor.h"
#include "myframe/context.h"
#include "myframe/log.h"

namespace myframe {

ContextManager::ContextManager() : ctx_count_(0) {
  LOG(INFO) << "ContextManager create";
  pthread_rwlock_init(&rw_, NULL);
}

ContextManager::~ContextManager() {
  LOG(INFO) << "ContextManager deconstruct";
  pthread_rwlock_destroy(&rw_);
}

bool ContextManager::RegContext(std::shared_ptr<Context> ctx) {
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

std::shared_ptr<Context> ContextManager::GetContext(
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

void ContextManager::PrintWaitQueue() {
  DLOG(INFO) << "cur wait queue actor:";
  auto it = wait_queue_.begin();
  while (it != wait_queue_.end()) {
    auto ctx = it->lock();
    if (ctx == nullptr) {
      LOG(ERROR) << "context is nullptr";
      continue;
    }
    DLOG(INFO) << "---> " << ctx->Print();
    ++it;
  }
}

std::shared_ptr<Context> ContextManager::GetContextWithMsg() {
  if (wait_queue_.empty()) {
    return nullptr;
  }

  std::vector<std::shared_ptr<Context>> in_runing_context;
  std::shared_ptr<Context> ret = nullptr;
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

void ContextManager::PushContext(std::shared_ptr<Context> ctx) {
  if (ctx->IsInWaitQueue()) {
    DLOG(INFO) << ctx->Print() << " already in wait queue, return";
    PrintWaitQueue();
    return;
  }
  ctx->SetWaitQueueFlag(true);
  wait_queue_.push_back(ctx);
  PrintWaitQueue();
}

}  // namespace myframe
