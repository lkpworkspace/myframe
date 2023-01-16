/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include "myframe/event_conn.h"

#include <glog/logging.h>

#include "myframe/common.h"
#include "myframe/msg.h"

namespace myframe {

int EventConn::GetFd() const { return cmd_channel_.GetMainFd(); }

EventType EventConn::GetType() { return EventType::kEventConn; }

Mailbox* EventConn::GetMailbox() {
  return &mailbox_;
}

CmdChannel* EventConn::GetCmdChannel() {
  return &cmd_channel_;
}

int EventConn::Send(
  const std::string& dst,
  std::shared_ptr<Msg> msg) {
  conn_type_ = EventConnType::kSend;
  mailbox_.SendClear();
  mailbox_.Send(dst, msg);
  cmd_channel_.SendToMain(Cmd::kRun);
  Cmd cmd;
  return cmd_channel_.RecvFromMain(&cmd);
}

const std::shared_ptr<const Msg> EventConn::SendRequest(
  const std::string& dst,
  std::shared_ptr<Msg> req) {
  conn_type_ = EventConnType::kSendReq;
  mailbox_.SendClear();
  mailbox_.Send(dst, req);
  cmd_channel_.SendToMain(Cmd::kRunWithMsg);
  Cmd cmd;
  cmd_channel_.RecvFromMain(&cmd);
  if (mailbox_.RecvEmpty()) {
    return nullptr;
  }
  auto msg = mailbox_.PopRecv();
  mailbox_.RecvClear();
  return msg;
}

}  // namespace myframe
