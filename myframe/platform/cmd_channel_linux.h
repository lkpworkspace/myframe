/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>

#include <memory>

#include <glog/logging.h>

#include "myframe/common.h"
#include "myframe/export.h"
#include "myframe/macros.h"
#include "myframe/event.h"
#include "myframe/cmd_channel.h"

namespace myframe {

class CmdChannelLinux final : public CmdChannel {
 public:
  explicit CmdChannelLinux(std::shared_ptr<Poller>);
  virtual ~CmdChannelLinux();

  ev_handle_t GetOwnerHandle() const override;
  ev_handle_t GetMainHandle() const override;

  int SendToOwner(const Cmd& cmd) override;
  int RecvFromOwner(Cmd* cmd) override;

  int SendToMain(const Cmd& cmd) override;
  int RecvFromMain(Cmd* cmd, int timeout_ms = -1) override;

 private:
  void CreateSockpair();
  void CloseSockpair();

  bool SetSockRecvTimeout(int fd, int timeout_ms) const;
  bool SetNonblockFd(int fd, bool b) const;
  bool IsBlockFd(int fd) const;

  ev_handle_t sockpair_[2] {-1, -1};

  DISALLOW_COPY_AND_ASSIGN(CmdChannelLinux)
};

CmdChannelLinux::CmdChannelLinux(std::shared_ptr<Poller> poller)
  : CmdChannel(poller) {
  CreateSockpair();
}

CmdChannelLinux::~CmdChannelLinux() {
  CloseSockpair();
}

ev_handle_t CmdChannelLinux::GetOwnerHandle() const {
  return sockpair_[0];
}

ev_handle_t CmdChannelLinux::GetMainHandle() const {
  return sockpair_[1];
}

void CmdChannelLinux::CreateSockpair() {
  int res = -1;
  res = socketpair(AF_UNIX, SOCK_DGRAM, 0, sockpair_);
  if (res) {
    LOG(ERROR) << "create sockpair failed";
    return;
  }
  if (!SetNonblockFd(sockpair_[0], false)) {
    LOG(ERROR) << "set sockpair[0] block failed, " << strerror(errno);
    return;
  }
  if (!SetNonblockFd(sockpair_[1], false)) {
    LOG(ERROR) << "set sockpair[1] block failed, " << strerror(errno);
    return;
  }
}

void CmdChannelLinux::CloseSockpair() {
  if (close(sockpair_[0])) {
    LOG(ERROR) << "close sockpair[0]: " << strerror(errno);
  }
  if (close(sockpair_[1])) {
    LOG(ERROR) << "close sockpair[1]: " << strerror(errno);
  }
}

int CmdChannelLinux::SendToOwner(const Cmd& cmd) {
  char cmd_char = static_cast<char>(cmd);
  int ret = write(sockpair_[1], &cmd_char, 1);
  if (ret < 0) {
    LOG(ERROR) << "write 1 cmd failed, " << strerror(errno);
  }
  return ret;
}

int CmdChannelLinux::RecvFromOwner(Cmd* cmd) {
  char cmd_char;
  int ret = read(sockpair_[1], &cmd_char, 1);
  if (ret < 0) {
    LOG(ERROR) << "read 1 cmd failed, " << strerror(errno);
    return ret;
  }
  *cmd = static_cast<Cmd>(cmd_char);
  return ret;
}

int CmdChannelLinux::RecvFromMain(Cmd* cmd, int timeout_ms) {
  (void)timeout_ms;
  if (timeout_ms < 0) {
    // block
    if (!IsBlockFd(sockpair_[0])) {
      SetNonblockFd(sockpair_[0], false);
    }
  } else if (timeout_ms == 0) {
    // nonblock
    if (IsBlockFd(sockpair_[0])) {
      SetNonblockFd(sockpair_[0], true);
    }
  } else {
    // timeout
    SetSockRecvTimeout(sockpair_[0], timeout_ms);
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

int CmdChannelLinux::SendToMain(const Cmd& cmd) {
  char cmd_char = static_cast<char>(cmd);
  int ret = write(sockpair_[0], &cmd_char, 1);
  if (ret < 0) {
    LOG(ERROR) << "write 0 cmd failed, " << strerror(errno);
  }
  return ret;
}

bool CmdChannelLinux::SetSockRecvTimeout(int fd, int timeout_ms) const {
  struct timeval timeout = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
  return 0 == setsockopt(
    fd, SOL_SOCKET, SO_RCVTIMEO,
    reinterpret_cast<const char*>(&timeout),
    sizeof(timeout));
}

bool CmdChannelLinux::SetNonblockFd(int fd, bool b) const {
  int flags = fcntl(fd, F_GETFL, 0);
  if (b) {
    flags |= O_NONBLOCK;
  } else {
    flags &= ~O_NONBLOCK;
  }
  return fcntl(fd, F_SETFL, flags) != -1;
}

bool CmdChannelLinux::IsBlockFd(int fd) const {
  int flags = fcntl(fd, F_GETFL, 0);
  return !(flags & O_NONBLOCK);
}

}  // namespace myframe
