/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <memory>

#include "myframe/common.h"
#include "myframe/worker.h"

namespace myframe {

class Msg;
class ActorContext;
class WorkerCommon final : public Worker {
  friend class App;

 public:
  WorkerCommon() = default;
  virtual ~WorkerCommon();

  void Run() override;
  void Init() override;
  void Exit() override;

  EventType GetType() override {
    return EventType::kWorkerCommon;
  }

  void SetActorContext(std::shared_ptr<ActorContext> context) {
    context_ = context;
  }
  std::shared_ptr<ActorContext> GetActorContext() {
    return context_.lock();
  }

 private:
  /* 工作线程消息处理 */
  int Work();
  /* 工作线程进入空闲链表之前进行的操作 */
  void Idle();

  /// 当前执行actor的指针
  std::weak_ptr<ActorContext> context_;
};

}  // namespace myframe
