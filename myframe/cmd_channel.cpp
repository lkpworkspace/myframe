/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/cmd_channel.h"
#include "myframe/platform.h"

#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
  #ifdef MYFRAME_USE_CV
    #include "myframe/platform/cmd_channel_generic.h"
  #else
    #include "myframe/platform/cmd_channel_linux.h"
  #endif
#elif defined(MYFRAME_OS_WINDOWS)
  #ifdef MYFRAME_USE_CV
    #include "myframe/platform/cmd_channel_generic.h"
  #else
    #error "Windows support conditional variables only,"
      " set MYFRAME_USE_CV to enable"
  #endif
#else
#error "Unsupported platform"
#endif

namespace myframe {

std::shared_ptr<CmdChannel> CmdChannel::Create(
    std::shared_ptr<Poller> poller) {
#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
  #ifdef MYFRAME_USE_CV
    return std::make_shared<CmdChannelGeneric>(poller);
  #else
    return std::make_shared<CmdChannelLinux>(poller);
  #endif
#elif defined(MYFRAME_OS_WINDOWS)
  #ifdef MYFRAME_USE_CV
    return std::make_shared<CmdChannelGeneric>(poller);
  #else
    #error "Windows support conditional variables only,"
      " set MYFRAME_USE_CV to enable"
  #endif
#else
#error "Unsupported platform"
#endif
}

}  // namespace myframe
