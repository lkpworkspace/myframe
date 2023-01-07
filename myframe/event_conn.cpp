/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/event_conn.h"

#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <glog/logging.h>

#include "myframe/common.h"
#include "myframe/msg.h"

namespace myframe {

EventConn::EventConn() { CreateSockPair(); }

EventConn::~EventConn() { CloseSockPair(); }

int EventConn::GetFd() { return sock_pair_[1]; }

EventType EventConn::GetType() { return EventType::EVENT_CONN; }

unsigned int EventConn::ListenEpollEventType() { return EPOLLIN; }

void EventConn::RetEpollEventType(uint32_t ev) { ev = ev; }

std::shared_ptr<Msg> EventConn::SendRequest(const std::string& dst,
                                                std::shared_ptr<Msg> req) {
  req->SetSrc(ev_conn_name_);
  req->SetDst(dst);
  send_.clear();
  send_.emplace_back(req);
  SendCmdToMain(WorkerCmd::IDLE);
  WorkerCmd cmd;
  RecvCmdFromMain(&cmd);
  if (recv_.empty()) {
    return nullptr;
  }
  auto msg = recv_.front();
  recv_.clear();
  return msg;
}

int EventConn::SendCmdToWorker(const WorkerCmd& cmd) {
  char cmd_char = static_cast<char>(cmd);
  return write(sock_pair_[1], &cmd_char, 1);
}

int EventConn::RecvCmdFromWorker(WorkerCmd* cmd) {
  char cmd_char;
  int ret = read(sock_pair_[1], &cmd_char, 1);
  *cmd = (WorkerCmd)cmd_char;
  return ret;
}

int EventConn::RecvCmdFromMain(WorkerCmd* cmd, int timeout_ms) {
  if (timeout_ms < 0) {
    // block
    if (!Common::IsBlockFd(sock_pair_[0])) {
      Common::SetNonblockFd(sock_pair_[0], false);
    }
  } else if (timeout_ms == 0) {
    // nonblock
    if (Common::IsBlockFd(sock_pair_[0])) {
      Common::SetNonblockFd(sock_pair_[0], true);
    }
  } else {
    // timeout
    Common::SetSockRecvTimeout(sock_pair_[0], timeout_ms);
  }
  char cmd_char;
  int ret = read(sock_pair_[0], &cmd_char, 1);
  *cmd = (WorkerCmd)cmd_char;
  return ret;
}

int EventConn::SendCmdToMain(const WorkerCmd& cmd) {
  char cmd_char = static_cast<char>(cmd);
  return write(sock_pair_[0], &cmd_char, 1);
}

bool EventConn::CreateSockPair() {
  int res = -1;
  bool ret = true;

  res = socketpair(AF_UNIX, SOCK_DGRAM, 0, sock_pair_);
  if (res == -1) {
    LOG(ERROR) << "Worker create sockpair failed";
    return false;
  }
  ret = Common::SetNonblockFd(sock_pair_[0], false);
  if (!ret) {
    LOG(ERROR) << "Worker set sockpair[0] block failed";
    return ret;
  }
  ret = Common::SetNonblockFd(sock_pair_[1], false);
  if (!ret) {
    LOG(ERROR) << "Worker set sockpair[1] block failed";
    return ret;
  }
  return ret;
}

void EventConn::CloseSockPair() {
  if (-1 == close(sock_pair_[0])) {
    LOG(ERROR) << "Worker close sockpair[0]: " << strerror(errno);
  }
  if (-1 == close(sock_pair_[1])) {
    LOG(ERROR) << "Worker close sockpair[1]: " << strerror(errno);
  }
}

void EventConn::SetEvConnName(const std::string& name) {
  ev_conn_name_ = name;
}

std::string EventConn::GetEvConnName() { return ev_conn_name_; }

}  // namespace myframe
