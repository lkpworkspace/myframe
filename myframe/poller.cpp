/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/poller.h"
#include "myframe/platform.h"

#ifdef MYFRAME_USE_CV
#include "myframe/platform/poller_generic.h"
#else
  #if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
  #include "myframe/platform/poller_linux.h"
  #else
  #include "myframe/platform/poller_generic.h"
  #endif
#endif

namespace myframe {

std::shared_ptr<Poller> Poller::Create() {
#ifdef MYFRAME_USE_CV
  return std::make_shared<PollerGeneric>();
#else
  #if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
    return std::make_shared<PollerLinux>();
  #else
    return std::make_shared<PollerGeneric>();
  #endif
#endif
}

}  // namespace myframe
