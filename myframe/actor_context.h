/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <list>
#include <memory>
#include <string>

#include <jsoncpp/json/json.h>

#include "myframe/macros.h"
#include "myframe/mailbox.h"

namespace myframe {

class App;
class Msg;
class Actor;
class WorkerCommon;
class ActorContext final : public std::enable_shared_from_this<ActorContext> {
  friend std::ostream& operator<<(std::ostream& out, const ActorContext& ctx);
  friend class ActorContextManager;
  friend class WorkerCommon;
  friend class App;

 public:
  ActorContext(std::shared_ptr<App> app, std::shared_ptr<Actor> actor);
  virtual ~ActorContext();

  int Init(const char* param);

  Mailbox* GetMailbox();

  /* 工作线程调用回调让actor去处理消息 */
  void Proc(const std::shared_ptr<const Msg>& msg);

  void SetRuningFlag(bool in_worker) { in_worker_ = in_worker; }
  bool IsRuning() { return in_worker_; }

  void SetWaitQueueFlag(bool in_wait_queue) { in_wait_que_ = in_wait_queue; }
  bool IsInWaitQueue() { return in_wait_que_; }

  std::shared_ptr<Actor> GetActor() { return actor_; }
  std::shared_ptr<App> GetApp();

  const Json::Value* GetConfig() const { return &config_; }
  void SetConfig(const Json::Value& conf) { config_ = conf; }

 private:
  Mailbox mailbox_;
  /* 该actor的是否在工作线程的标志 */
  bool in_worker_;
  /* actor是否在消息队列中 */
  bool in_wait_que_;
  std::shared_ptr<Actor> actor_;
  std::weak_ptr<App> app_;
  Json::Value config_{ Json::Value::null };

  DISALLOW_COPY_AND_ASSIGN(ActorContext)
};

std::ostream& operator<<(std::ostream& out, const ActorContext& ctx);

}  // namespace myframe
