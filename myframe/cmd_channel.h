/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include "myframe/macros.h"
#include "myframe/event.h"

namespace myframe {

class CmdChannel final {
 public:
  enum class Cmd : char {
    kQuit = 'q',          ///< 退出
    kIdle = 'i',          ///< 空闲
    kWaitForMsg = 'w',    ///< 等待消息
    kRun = 'r',           ///< 运行
    kRunWithMsg = 'm',    ///< 运行(有消息)
  };

  CmdChannel();
  virtual ~CmdChannel();

  ev_handle_t GetOwnerHandle() const;
  ev_handle_t GetMainHandle() const;

  int SendToOwner(const Cmd& cmd);
  int RecvFromOwner(Cmd* cmd);

  int SendToMain(const Cmd& cmd);
  int RecvFromMain(Cmd* cmd, int timeout_ms = -1);

 private:
  void CreateSockpair();
  void CloseSockpair();
  ev_handle_t sockpair_[2] {-1, -1};

  DISALLOW_COPY_AND_ASSIGN(CmdChannel)
};

}  // namespace myframe
