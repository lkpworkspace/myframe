/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include "myframe/macros.h"

namespace myframe {

enum class Cmd : char {
  kQuit = 'q',          ///< 退出
  kIdle = 'i',          ///< 空闲
  kWaitForMsg = 'w',    ///< 等待消息
  kRun = 'r',           ///< 运行
  kRunWithMsg = 'm',    ///< 运行
};

class CmdChannel final {
 public:
  CmdChannel();
  virtual ~CmdChannel();

  int GetOwnerFd() const;
  int GetMainFd() const;

  int SendToOwner(const Cmd& cmd);
  int RecvFromOwner(Cmd* cmd);

  int SendToMain(const Cmd& cmd);
  int RecvFromMain(Cmd* cmd, int timeout_ms = -1);

 private:
  void CreateSockpair();
  void CloseSockpair();
  int sockpair_[2] {-1, -1};

  DISALLOW_COPY_AND_ASSIGN(CmdChannel)
};

}  // namespace myframe
