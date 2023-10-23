/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <memory>
#include <string>

#include "myframe/macros.h"
#include "myframe/event.h"
#include "myframe/mailbox.h"
#include "myframe/cmd_channel.h"
#include "myframe/poller.h"

namespace myframe {

class Msg;
class EventConnManager;
class EventConn final : public Event {
  friend class App;
  friend class EventConnManager;

 public:
  enum class Type : char {
    kSendReq,
    kSend,
  };

  explicit EventConn(std::shared_ptr<Poller>);

  ev_handle_t GetHandle() const override;
  Event::Type GetType() const override;
  std::string GetName() const override;

  EventConn::Type GetConnType() { return conn_type_; }

  int Send(
    const std::string& dst,
    std::shared_ptr<Msg> msg);

  const std::shared_ptr<const Msg> SendRequest(
    const std::string& dst,
    std::shared_ptr<Msg> req);

 private:
  Mailbox* GetMailbox();
  CmdChannel* GetCmdChannel();

  std::shared_ptr<CmdChannel> cmd_channel_;
  Mailbox mailbox_;
  EventConn::Type conn_type_{ EventConn::Type::kSendReq };

  DISALLOW_COPY_AND_ASSIGN(EventConn)
};

}  // namespace myframe
