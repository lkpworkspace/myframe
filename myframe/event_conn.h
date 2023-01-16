/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <memory>
#include <string>

#include "myframe/event.h"
#include "myframe/mailbox.h"
#include "myframe/cmd_channel.h"

namespace myframe {

enum class EventConnType : char {
  kSendReq,
  kSend,
};

class Msg;
class EventConnManager;
class EventConn final : public Event {
  friend class App;
  friend class EventConnManager;

 public:
  int GetFd() const override;
  EventType GetType() override;

  EventConnType GetConnType() { return conn_type_; }

  int Send(
    const std::string& dst,
    std::shared_ptr<Msg> msg);

  const std::shared_ptr<const Msg> SendRequest(
    const std::string& dst,
    std::shared_ptr<Msg> req);

 private:
  Mailbox* GetMailbox();
  CmdChannel* GetCmdChannel();

  CmdChannel cmd_channel_;
  Mailbox mailbox_;
  EventConnType conn_type_{ EventConnType::kSendReq };
};

}  // namespace myframe
