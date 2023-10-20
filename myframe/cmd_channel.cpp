/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/cmd_channel.h"
#include "myframe/platform.h"

#ifdef MYFRAME_USE_CV
#include "myframe/platform/cmd_channel_generic.h"
#else
  #ifdef MYFRAME_OS_LINUX
  #include "myframe/platform/cmd_channel_linux.h"
  #else
  #include "myframe/platform/cmd_channel_generic.h"
  #endif
#endif

namespace myframe {

std::shared_ptr<CmdChannel> CmdChannel::Create(
    std::shared_ptr<Poller> poller) {
#ifdef MYFRAME_USE_CV
  return std::make_shared<CmdChannelGeneric>(poller);
#else
  #ifdef MYFRAME_OS_LINUX
    return std::make_shared<CmdChannelLinux>(poller);
  #else
    return std::make_shared<CmdChannelGeneric>(poller);
  #endif
#endif
}

}  // namespace myframe
