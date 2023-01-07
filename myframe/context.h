/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <list>
#include <memory>
#include <string>

namespace myframe {

class App;
class Msg;
class Actor;
class WorkerCommon;
class Context final : public std::enable_shared_from_this<Context> {
  friend class ContextManager;
  friend class WorkerCommon;
  friend class App;

 public:
  Context(std::shared_ptr<App> app, std::shared_ptr<Actor> mod);
  virtual ~Context();

  int Init(const char* param);

  /* actor将消息添加至发送队列中 */
  int SendMsg(std::shared_ptr<Msg> msg);

  /* 工作线程调用回调让actor去处理消息 */
  void Proc(const std::shared_ptr<const Msg>& msg);

  /* 主线程发送消息给该actor */
  void PushMsg(std::shared_ptr<Msg> msg) { recv_.emplace_back(msg); }

  /* 主线程获得该actor待处理消息链表 */
  std::list<std::shared_ptr<Msg>>& GetRecvMsgList() { return recv_; }

  /* 主线程获得该actor发送消息链表 */
  std::list<std::shared_ptr<Msg>>& GetDispatchMsgList() { return send_; }

  void SetRuningFlag(bool in_worker) { in_worker_ = in_worker; }
  bool IsRuning() { return in_worker_; }

  void SetWaitQueueFlag(bool in_wait_queue) { in_wait_que_ = in_wait_queue; }
  bool IsInWaitQueue() { return in_wait_que_; }

  std::shared_ptr<Actor> GetActor() { return actor_; }
  std::shared_ptr<App> GetApp();

  std::string Print();

 private:
  /* 主线程分发给actor的消息链表，主线程操作该链表 */
  std::list<std::shared_ptr<Msg>> recv_;
  /* actor发送给别的actor的消息链表，工作线程处理该actor时可以操作该链表,
   * 空闲时主线程操作该链表 */
  std::list<std::shared_ptr<Msg>> send_;
  /* 该actor的是否在工作线程的标志 */
  bool in_worker_;
  /* actor是否在消息队列中 */
  bool in_wait_que_;
  std::shared_ptr<Actor> actor_;
  std::weak_ptr<App> app_;
};

}  // namespace myframe
