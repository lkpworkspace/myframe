/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/poller.h"
#include "myframe/platform.h"

#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
  #ifdef MYFRAME_USE_CV
    #include "myframe/platform/poller_generic.h"
  #else
    #include "myframe/platform/poller_linux.h"
  #endif
#elif defined(MYFRAME_OS_WINDOWS) || defined(MYFRAME_OS_MACOSX)
  #ifdef MYFRAME_USE_CV
    #include "myframe/platform/poller_generic.h"
  #else
    #error "Support conditional variables only,"
      " set MYFRAME_USE_CV to enable"
  #endif
#else
#error "Unsupported platform"
#endif

namespace myframe {

std::shared_ptr<Poller> Poller::Create() {
#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
  #ifdef MYFRAME_USE_CV
    return std::make_shared<PollerGeneric>();
  #else
    return std::make_shared<PollerLinux>();
  #endif
#elif defined(MYFRAME_OS_WINDOWS) || defined(MYFRAME_OS_MACOSX)
  #ifdef MYFRAME_USE_CV
    return std::make_shared<PollerGeneric>();
  #else
    #error "Support conditional variables only,"
      " set MYFRAME_USE_CV to enable"
  #endif
#else
#error "Unsupported platform"
#endif
}

}  // namespace myframe
