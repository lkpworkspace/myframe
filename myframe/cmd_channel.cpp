/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/cmd_channel.h"
#include "myframe/platform.h"

#ifdef MYFRAME_OS_LINUX
#include "myframe/platform/cmd_channel_linux.h"
#else
#error "Platform not supported"
#endif

namespace myframe {

std::shared_ptr<CmdChannel> CmdChannel::Create() {
#ifdef MYFRAME_OS_LINUX
  return std::make_shared<CmdChannelLinux>();
#else
  return nullptr;
#endif
}

}  // namespace myframe
