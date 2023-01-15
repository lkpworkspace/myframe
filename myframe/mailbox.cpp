/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include "myframe/mailbox.h"
#include "myframe/common.h"
#include "myframe/msg.h"

namespace myframe {

const std::string& Mailbox::Addr() const {
  return addr_;
}

void Mailbox::SetAddr(const std::string& addr) {
  addr_ = addr;
}

int Mailbox::SendSize() const {
  return send_.size();
}

bool Mailbox::SendEmpty() const {
  return send_.empty();
}

void Mailbox::SendClear() {
  send_.clear();
}

void Mailbox::Send(std::shared_ptr<Msg> msg) {
  send_.emplace_back(msg);
}

void Mailbox::Send(
  const std::string& dst,
  std::shared_ptr<Msg> msg) {
  msg->SetSrc(addr_);
  msg->SetDst(dst);
  Send(msg);
}

void Mailbox::Send(
  const std::string& dst,
  const std::any& data) {
  auto msg = std::make_shared<Msg>();
  msg->SetAnyData(data);
  Send(dst, msg);
}

void Mailbox::Send(std::list<std::shared_ptr<Msg>>* msg_list) {
  ListAppend(&send_, msg_list);
}

std::list<std::shared_ptr<Msg>>* Mailbox::GetSendList() {
  return &send_;
}

int Mailbox::RecvSize() const {
  return recv_.size();
}

bool Mailbox::RecvEmpty() const {
  return recv_.empty();
}

void Mailbox::RecvClear() {
  recv_.clear();
}

void Mailbox::Recv(std::shared_ptr<Msg> msg) {
  recv_.emplace_back(msg);
}

void Mailbox::Recv(std::list<std::shared_ptr<Msg>>* msg_list) {
  ListAppend(&recv_, msg_list);
}

const std::shared_ptr<const Msg> Mailbox::PopRecv() {
  if (recv_.empty()) {
    return nullptr;
  }
  auto msg = recv_.front();
  recv_.pop_front();
  return msg;
}

std::list<std::shared_ptr<Msg>>* Mailbox::GetRecvList() {
  return &recv_;
}

std::ostream& operator<<(std::ostream& out, const Mailbox& mailbox) {
  out << mailbox.Addr() << " recv " << mailbox.RecvSize() << ", "
    << " send " << mailbox.SendSize();
  return out;
}

};  // namespace myframe
