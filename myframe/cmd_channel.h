/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <memory>

#include "myframe/export.h"
#include "myframe/macros.h"
#include "myframe/event.h"

namespace myframe {

class MYFRAME_EXPORT CmdChannel {
 public:
  enum class Cmd : char {
    kQuit = 'q',          ///< 退出
    kIdle = 'i',          ///< 空闲
    kWaitForMsg = 'w',    ///< 等待消息
    kRun = 'r',           ///< 运行
    kRunWithMsg = 'm',    ///< 运行(有消息)
  };
  CmdChannel() = default;
  virtual ~CmdChannel() = default;

  static std::shared_ptr<CmdChannel> Create();

  virtual ev_handle_t GetOwnerHandle() const = 0;
  virtual ev_handle_t GetMainHandle() const = 0;

  virtual int SendToOwner(const Cmd& cmd) = 0;
  virtual int RecvFromOwner(Cmd* cmd) = 0;

  virtual int SendToMain(const Cmd& cmd) = 0;
  virtual int RecvFromMain(Cmd* cmd, int timeout_ms = -1) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(CmdChannel)
};

}  // namespace myframe
