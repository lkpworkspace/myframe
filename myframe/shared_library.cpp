/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include "myframe/shared_library.h"
#include "myframe/platform.h"

#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
#include "myframe/platform/shared_library_linux.h"
#else
#error "Platform not supported"
#endif

namespace myframe {

std::shared_ptr<SharedLibrary> SharedLibrary::Create() {
#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
  return std::make_shared<SharedLibraryLinux>();
#else
  return nullptr;
#endif
}

}  // namespace myframe
