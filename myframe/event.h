/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <memory>
#include <string>

#include "myframe/config.h"
#include "myframe/export.h"
#include "myframe/platform.h"

namespace myframe {

#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
  #ifdef MYFRAME_USE_CV
    typedef void* ev_handle_t;
  #else
    typedef int ev_handle_t;
  #endif
#elif defined(MYFRAME_OS_WINDOWS) || defined(MYFRAME_OS_MACOSX)
  #ifdef MYFRAME_USE_CV
    typedef void* ev_handle_t;
  #else
    #error "Support conditional variables only,"
      " set MYFRAME_USE_CV to enable"
  #endif
#else
#error "Unsupported platform"
#endif

class MYFRAME_EXPORT Event {
 public:
  enum class Type : int {
    kWorkerCommon,
    kWorkerTimer,
    kWorkerUser,
    kEventConn,
  };

  Event() = default;
  virtual ~Event() = default;

  /* 事件类型 */
  virtual Type GetType() const { return Type::kWorkerUser; }

  /* 事件句柄 */
  virtual ev_handle_t GetHandle() const = 0;

  /* 事件名称 */
  virtual std::string GetName() const = 0;

#if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
  #ifdef MYFRAME_USE_CV
    static const ev_handle_t DEFAULT_EV_HANDLE;
  #else
    static const ev_handle_t DEFAULT_EV_HANDLE{-1};
  #endif
#elif defined(MYFRAME_OS_WINDOWS) || defined(MYFRAME_OS_MACOSX)
  #ifdef MYFRAME_USE_CV
    static const ev_handle_t DEFAULT_EV_HANDLE;
  #else
    #error "Support conditional variables only,"
      " set MYFRAME_USE_CV to enable"
  #endif
#else
#error "Unsupported platform"
#endif
};

}  // namespace myframe
