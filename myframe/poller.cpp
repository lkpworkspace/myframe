/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/poller.h"
#include "myframe/platform.h"

#ifdef MYFRAME_OS_LINUX
#include "myframe/platform/poller_linux.h"
#else
#error "Platform not supported"
#endif

namespace myframe {

std::shared_ptr<Poller> Poller::Create() {
#ifdef MYFRAME_OS_LINUX
  return std::make_shared<PollerLinux>();
#else
  return nullptr;
#endif
}

}  // namespace myframe
