/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <memory>

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

  Event::Type GetType() override {
    return Event::Type::kWorkerCommon;
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
