/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/mailbox.h"
#include <utility>

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
  send_.push_back(std::move(msg));
}

void Mailbox::Send(
  const std::string& dst,
  std::shared_ptr<Msg> msg) {
  msg->SetSrc(addr_);
  msg->SetDst(dst);
  Send(std::move(msg));
}

void Mailbox::Send(
  const std::string& dst,
  const std::any& data) {
  auto msg = std::make_shared<Msg>();
  msg->SetAnyData(data);
  Send(dst, std::move(msg));
}

void Mailbox::Send(std::list<std::shared_ptr<Msg>>* msg_list) {
  send_.splice(send_.end(), *msg_list);
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
  if (pending_queue_size_ > 0) {
    for (; recv_.size() >= static_cast<std::size_t>(pending_queue_size_);) {
      recv_.pop_front();
    }
  }
  recv_.push_back(std::move(msg));
}

const std::shared_ptr<const Msg> Mailbox::PopRecv() {
  if (recv_.empty()) {
    return nullptr;
  }
  auto msg = recv_.front();
  recv_.pop_front();
  return msg;
}

void Mailbox::MoveToRun() {
  if (run_queue_size_ > 0) {
    auto it = recv_.begin();
    for (size_t i = 0;
        i < static_cast<std::size_t>(run_queue_size_) && it != recv_.end();
        ++i) {
      ++it;
    }
    run_.splice(run_.begin(), recv_, recv_.begin(), it);
    return;
  }
  run_.splice(run_.end(), recv_);
}

bool Mailbox::RunEmpty() const {
  return run_.empty();
}

int Mailbox::RunSize() const {
  return run_.size();
}

const std::shared_ptr<const Msg> Mailbox::PopRun() {
  if (run_.empty()) {
    return nullptr;
  }
  auto msg = run_.front();
  run_.pop_front();
  return msg;
}

void Mailbox::SetPendingQueueSize(int sz) {
  pending_queue_size_ = sz;
}

int Mailbox::GetPendingQueueSize() const {
  return pending_queue_size_;
}

void Mailbox::SetRunQueueSize(int sz) {
  run_queue_size_ = sz;
}

int Mailbox::GetRunQueueSize() const {
  return run_queue_size_;
}

std::list<std::shared_ptr<Msg>>* Mailbox::GetRecvList() {
  return &recv_;
}

std::ostream& operator<<(std::ostream& out, const Mailbox& mailbox) {
  out << mailbox.Addr() << " recv " << mailbox.RecvSize()
    << ", send " << mailbox.SendSize()
    << ", run " << mailbox.RunSize();
  return out;
}

}  // namespace myframe
