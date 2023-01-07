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
class Context;
class WorkerCommon final : public Worker {
  friend class App;

 public:
  WorkerCommon();
  ~WorkerCommon();

  /// override MyWorker virtual method
  void Run() override;
  void OnInit() override;
  void OnExit() override;

  /// override Event virtual method
  EventType GetType() { return EventType::WORKER_COMMON; }

  void SetContext(std::shared_ptr<Context> context) { context_ = context; }
  std::shared_ptr<Context> GetContext() {
    return (context_.expired() ? nullptr : context_.lock());
  }

 private:
  /* 工作线程消息处理 */
  int Work();
  /* 工作线程进入空闲链表之前进行的操作 */
  void Idle();

  /// 当前执行actor的指针
  std::weak_ptr<Context> context_;
};

}  // namespace myframe
