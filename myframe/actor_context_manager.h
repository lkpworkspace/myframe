/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <pthread.h>
#include <stdint.h>

#include <list>
#include <memory>
#include <string>
#include <unordered_map>

#include "myframe/macros.h"

namespace myframe {
class Msg;
class ActorContext;
class ActorContextManager final {
 public:
  ActorContextManager();
  virtual ~ActorContextManager();

  /* 注册actor */
  bool RegContext(std::shared_ptr<ActorContext> ctx);

  void DispatchMsg(std::shared_ptr<Msg> msg);

  /* 获得一个待处理的actor */
  std::shared_ptr<ActorContext> GetContextWithMsg();

 private:
  /* 获得actor名对应的actor */
  std::shared_ptr<ActorContext> GetContext(const std::string& actor_name);
  /* 将有消息的actor放入链表 */
  void PushContext(std::shared_ptr<ActorContext> ctx);
  void PrintWaitQueue();

  /// 当前注册actor数量
  uint32_t ctx_count_;
  /// 待处理actor链表
  std::list<std::weak_ptr<ActorContext>> wait_queue_;
  /// 读写锁
  pthread_rwlock_t rw_;
  /// key: context name, value: context
  std::unordered_map<std::string, std::shared_ptr<ActorContext>> ctxs_;

  DISALLOW_COPY_AND_ASSIGN(ActorContextManager)
};

}  // namespace myframe
