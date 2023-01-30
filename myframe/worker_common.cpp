/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/worker_common.h"

#include <glog/logging.h>

#include "myframe/msg.h"
#include "myframe/actor_context.h"

namespace myframe {

WorkerCommon::WorkerCommon() {}

WorkerCommon::~WorkerCommon() {}

void WorkerCommon::Idle() {
  if (!context_.expired()) {
    context_.reset();
  }
}

void WorkerCommon::Run() {
  DispatchMsg();
  Work();
}

void WorkerCommon::Init() {
  LOG(INFO) << "Worker " << GetWorkerName() << " init";
}

void WorkerCommon::Exit() {
  LOG(INFO) << "Worker " << GetWorkerName() << " exit";
}

int WorkerCommon::Work() {
  auto ctx = context_.lock();
  if (ctx == nullptr) {
    LOG(ERROR) << "context is nullptr";
    return -1;
  }
  while (!GetMailbox()->RecvEmpty()) {
    ctx->Proc(GetMailbox()->PopRecv());
  }

  return 0;
}

}  // namespace myframe
