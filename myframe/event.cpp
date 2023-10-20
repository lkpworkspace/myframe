/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/event.h"

namespace myframe {

#ifdef MYFRAME_USE_CV
  const ev_handle_t Event::DEFAULT_EV_HANDLE{nullptr};
#else
  #if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
    // do nothing
  #else
    const ev_handle_t Event::DEFAULT_EV_HANDLE{nullptr};
  #endif
#endif

}  // namespace myframe