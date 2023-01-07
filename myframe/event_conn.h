/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <list>
#include <memory>
#include <string>

#include "myframe/event.h"
#include "myframe/worker.h"

namespace myframe {

class Msg;
class EventConnManager;
class EventConn final : public Event {
  friend class App;
  friend class EventConnManager;

 public:
  EventConn();
  virtual ~EventConn();

  int GetFd() override;
  EventType GetType() override;
  unsigned int ListenEpollEventType() override;
  void RetEpollEventType(uint32_t ev) override;

  std::shared_ptr<Msg> SendRequest(const std::string& dst,
                                     std::shared_ptr<Msg> req);

 private:
  std::string GetEvConnName();
  void SetEvConnName(const std::string& name);

  int SendCmdToWorker(const WorkerCmd& cmd);
  int RecvCmdFromWorker(WorkerCmd* cmd);

  int RecvCmdFromMain(WorkerCmd* cmd, int timeout_ms = -1);
  int SendCmdToMain(const WorkerCmd& cmd);

  bool CreateSockPair();
  void CloseSockPair();
  int sock_pair_[2];

  /// 接收消息队列
  std::list<std::shared_ptr<Msg>> recv_;
  /// 发送消息队列
  std::list<std::shared_ptr<Msg>> send_;

  std::string ev_conn_name_{ "" };
};

}  // namespace myframe
