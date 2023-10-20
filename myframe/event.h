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

#ifdef MYFRAME_USE_CV
  typedef void* ev_handle_t;
#else
  #if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
    typedef int ev_handle_t;
  #else
    typedef void* ev_handle_t;
  #endif
#endif

class MYFRAME_EXPORT Event : public std::enable_shared_from_this<Event> {
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

#ifdef MYFRAME_USE_CV
  static const ev_handle_t DEFAULT_EV_HANDLE;
#else
  #if defined(MYFRAME_OS_LINUX) || defined(MYFRAME_OS_ANDROID)
    static const ev_handle_t DEFAULT_EV_HANDLE{-1};
  #else
    static const ev_handle_t DEFAULT_EV_HANDLE;
  #endif
#endif
};

}  // namespace myframe
