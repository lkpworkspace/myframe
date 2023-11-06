/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/event_conn.h"

#include <glog/logging.h>

#include "myframe/msg.h"

namespace myframe {

EventConn::EventConn(std::shared_ptr<Poller> poller) {
  cmd_channel_ = CmdChannel::Create(poller);
}

ev_handle_t EventConn::GetHandle() const {
  return cmd_channel_->GetMainHandle();
}

Event::Type EventConn::GetType() const {
  return Event::Type::kEventConn;
}

std::string EventConn::GetName() const {
  return mailbox_.Addr();
}

Mailbox* EventConn::GetMailbox() {
  return &mailbox_;
}

CmdChannel* EventConn::GetCmdChannel() {
  return cmd_channel_.get();
}

int EventConn::Send(std::shared_ptr<Msg> msg) {
  conn_type_ = EventConn::Type::kSend;
  mailbox_.SendClear();
  if (msg->GetSrc().empty()) {
    msg->SetSrc(mailbox_.Addr());
  }
  if (msg->GetDst().empty()) {
    return -1;
  }
  mailbox_.Send(msg);
  cmd_channel_->SendToMain(CmdChannel::Cmd::kRun);
  CmdChannel::Cmd cmd;
  return cmd_channel_->RecvFromMain(&cmd);
}

const std::shared_ptr<const Msg> EventConn::SendRequest(
  std::shared_ptr<Msg> req) {
  conn_type_ = EventConn::Type::kSendReq;
  mailbox_.SendClear();
  if (req->GetSrc().empty()) {
    req->SetSrc(mailbox_.Addr());
  }
  if (req->GetDst().empty()) {
    return nullptr;
  }
  mailbox_.Send(req);
  cmd_channel_->SendToMain(CmdChannel::Cmd::kRunWithMsg);
  CmdChannel::Cmd cmd;
  cmd_channel_->RecvFromMain(&cmd);
  if (mailbox_.RecvEmpty()) {
    return nullptr;
  }
  auto msg = mailbox_.PopRecv();
  mailbox_.RecvClear();
  return msg;
}

}  // namespace myframe
