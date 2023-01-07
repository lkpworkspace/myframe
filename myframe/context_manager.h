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

namespace myframe {

class Context;
class ContextManager final {
 public:
  ContextManager();
  virtual ~ContextManager();

  /* 注册actor */
  bool RegContext(std::shared_ptr<Context> ctx);

  /* 获得actor名对应的actor */
  std::shared_ptr<Context> GetContext(const std::string& actor_name);

  /* 获得一个待处理的actor */
  std::shared_ptr<Context> GetContextWithMsg();

  /* 将有消息的actor放入链表 */
  void PushContext(std::shared_ptr<Context> ctx);

 private:
  void PrintWaitQueue();

  /// 当前注册actor数量
  uint32_t ctx_count_;
  /// 待处理actor链表
  std::list<std::weak_ptr<Context>> wait_queue_;
  /// 读写锁
  pthread_rwlock_t rw_;
  /// key: context name, value: context
  std::unordered_map<std::string, std::shared_ptr<Context>> ctxs_;
};

}  // namespace myframe
