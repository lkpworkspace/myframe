/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <string>
#include <list>
#include <memory>
#include <any>

#include "myframe/export.h"

namespace myframe {

class Msg;
class MYFRAME_EXPORT Mailbox final {
  friend class ActorContext;
  friend class ActorContextManager;
  friend class WorkerContext;
  friend class WorkerContextManager;
  friend class EventConn;
  friend class EventConnManager;
  friend class App;

 public:
  /// 邮箱地址
  const std::string& Addr() const;

  /// 发件箱
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

  /// 信件处理
  void MoveToRun();
  bool RunEmpty() const;
  int RunSize() const;
  const std::shared_ptr<const Msg> PopRun();

  /// 收件箱
  int RecvSize() const;
  bool RecvEmpty() const;

 private:
  /// 收件箱
  void RecvClear();
  void Recv(std::shared_ptr<Msg> msg);
  void Recv(std::list<std::shared_ptr<Msg>>* msg_list);
  const std::shared_ptr<const Msg> PopRecv();

  /// 设置邮箱地址
  void SetAddr(const std::string& addr);

  std::list<std::shared_ptr<Msg>>* GetSendList();
  std::list<std::shared_ptr<Msg>>* GetRecvList();

  std::string addr_;
  std::list<std::shared_ptr<Msg>> recv_;
  std::list<std::shared_ptr<Msg>> send_;
  std::list<std::shared_ptr<Msg>> run_;
};

MYFRAME_EXPORT std::ostream& operator<<(
  std::ostream& out, const Mailbox& mailbox);

}  // namespace myframe
