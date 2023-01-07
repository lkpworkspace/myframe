/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/worker_common.h"

#include "myframe/context.h"
#include "myframe/log.h"
#include "myframe/msg.h"

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

void WorkerCommon::OnInit() {
  Worker::OnInit();
  LOG(INFO) << "Worker " << GetWorkerName() << " init";
}

void WorkerCommon::OnExit() {
  Worker::OnExit();
  LOG(INFO) << "Worker " << GetWorkerName() << " exit";
}

int WorkerCommon::Work() {
  auto ctx = (context_.expired() ? nullptr : context_.lock());
  if (ctx == nullptr) {
    LOG(ERROR) << "context is nullptr";
    return -1;
  }
  while (RecvMsgListSize() > 0) {
    ctx->Proc(GetRecvMsg());
  }

  return 0;
}

}  // namespace myframe
