/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/event.h"

namespace myframe {

#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
  #ifdef MYFRAME_USE_CV
    const ev_handle_t Event::DEFAULT_EV_HANDLE{nullptr};
  #endif
#elif defined(MYFRAME_OS_WINDOWS)
  #ifdef MYFRAME_USE_CV
    const ev_handle_t Event::DEFAULT_EV_HANDLE{nullptr};
  #else
    #error "Windows support conditional variables only,"
      " set MYFRAME_USE_CV to enable"
  #endif
#else
#error "Unsupported platform"
#endif

}  // namespace myframe
