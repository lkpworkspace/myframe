/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include "myframe/cmd_channel.h"

#include <sys/socket.h>
#include <sys/types.h>

#include <glog/logging.h>

#include "myframe/common.h"

namespace myframe {

CmdChannel::CmdChannel() {
  CreateSockpair();
}

CmdChannel::~CmdChannel() {
  CloseSockpair();
}

ev_handle_t CmdChannel::GetOwnerHandle() const {
  return sockpair_[0];
}

ev_handle_t CmdChannel::GetMainHandle() const {
  return sockpair_[1];
}

void CmdChannel::CreateSockpair() {
  int res = -1;
  res = socketpair(AF_UNIX, SOCK_DGRAM, 0, sockpair_);
  if (res) {
    LOG(ERROR) << "create sockpair failed";
    return;
  }
  if (!Common::SetNonblockFd(sockpair_[0], false)) {
    LOG(ERROR) << "set sockpair[0] block failed, " << strerror(errno);
    return;
  }
  if (!Common::SetNonblockFd(sockpair_[1], false)) {
    LOG(ERROR) << "set sockpair[1] block failed, " << strerror(errno);
    return;
  }
}

void CmdChannel::CloseSockpair() {
  if (close(sockpair_[0])) {
    LOG(ERROR) << "close sockpair[0]: " << strerror(errno);
  }
  if (close(sockpair_[1])) {
    LOG(ERROR) << "close sockpair[1]: " << strerror(errno);
  }
}

int CmdChannel::SendToOwner(const Cmd& cmd) {
  char cmd_char = static_cast<char>(cmd);
  int ret = write(sockpair_[1], &cmd_char, 1);
  if (ret < 0) {
    LOG(ERROR) << "write 1 cmd failed, " << strerror(errno);
  }
  return ret;
}

int CmdChannel::RecvFromOwner(Cmd* cmd) {
  char cmd_char;
  int ret = read(sockpair_[1], &cmd_char, 1);
  if (ret < 0) {
    LOG(ERROR) << "read 1 cmd failed, " << strerror(errno);
    return ret;
  }
  *cmd = static_cast<Cmd>(cmd_char);
  return ret;
}

int CmdChannel::RecvFromMain(Cmd* cmd, int timeout_ms) {
  if (timeout_ms < 0) {
    // block
    if (!Common::IsBlockFd(sockpair_[0])) {
      Common::SetNonblockFd(sockpair_[0], false);
    }
  } else if (timeout_ms == 0) {
    // nonblock
    if (Common::IsBlockFd(sockpair_[0])) {
      Common::SetNonblockFd(sockpair_[0], true);
    }
  } else {
    // timeout
    Common::SetSockRecvTimeout(sockpair_[0], timeout_ms);
  }
  char cmd_char;
  int ret = read(sockpair_[0], &cmd_char, 1);
  if (ret < 0) {
    LOG(ERROR) << "read 0 cmd failed, " << strerror(errno);
    return ret;
  }
  *cmd = static_cast<Cmd>(cmd_char);
  return ret;
}

int CmdChannel::SendToMain(const Cmd& cmd) {
  char cmd_char = static_cast<char>(cmd);
  int ret = write(sockpair_[0], &cmd_char, 1);
  if (ret < 0) {
    LOG(ERROR) << "write 0 cmd failed, " << strerror(errno);
  }
  return ret;
}

}  // namespace myframe
