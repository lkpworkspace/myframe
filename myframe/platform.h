/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
  #define MYFRAME_OS_WINDOWS
#elif defined(linux) || defined(__linux) || defined(__linux__)
  #define MYFRAME_OS_LINUX
#elif defined(ANDROID) || defined(__ANDROID__)
  #define MYFRAME_OS_ANDROID
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
  #define MYFRAME_OS_MACOSX
#else
  #error Platform not supported by myframe.
#endif
