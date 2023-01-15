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
#include "myframe/mailbox.h"

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

  const std::shared_ptr<const Msg> SendRequest(
    const std::string& dst,
    std::shared_ptr<Msg> req);

 private:
  Mailbox* GetMailbox();
  int SendCmdToWorker(const WorkerCmd& cmd);
  int RecvCmdFromWorker(WorkerCmd* cmd);

  int RecvCmdFromMain(WorkerCmd* cmd, int timeout_ms = -1);
  int SendCmdToMain(const WorkerCmd& cmd);

  bool CreateSockPair();
  void CloseSockPair();
  int sock_pair_[2];

  Mailbox mailbox_;
};

}  // namespace myframe
