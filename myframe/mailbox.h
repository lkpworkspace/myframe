/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <string>
#include <list>
#include <memory>
#include <any>

namespace myframe {

class Msg;
class Mailbox final {
  friend class ActorContext;
  friend class WorkerContext;
  friend class EventConnManager;
  friend class App;

 public:
  /// 邮箱地址
  const std::string& Addr() const;

  /// 发件箱(适用于worker/actor)
  int SendSize() const;
  bool SendEmpty() const;
  void SendClear();
  void Send(std::shared_ptr<Msg> msg);
  void Send(
    const std::string& dst,
    std::shared_ptr<Msg> msg);
  void Send(
    const std::string& dst,
    const std::any& data);
  void Send(std::list<std::shared_ptr<Msg>>* msg_list);

  /// 收件箱(适用于worker)
  int RecvSize() const;
  bool RecvEmpty() const;
  void RecvClear();
  void Recv(std::shared_ptr<Msg> msg);
  void Recv(std::list<std::shared_ptr<Msg>>* msg_list);
  const std::shared_ptr<const Msg> PopRecv();

 private:
  /// 设置邮箱地址
  void SetAddr(const std::string& addr);

  std::list<std::shared_ptr<Msg>>* GetSendList();
  std::list<std::shared_ptr<Msg>>* GetRecvList();

  std::string addr_;
  std::list<std::shared_ptr<Msg>> recv_;
  std::list<std::shared_ptr<Msg>> send_;
};

std::ostream& operator<<(std::ostream& out, const Mailbox& mailbox);

};  // namespace myframe
