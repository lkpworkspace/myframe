/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <memory>

namespace myframe {

typedef int ev_handle_t;

class Event : public std::enable_shared_from_this<Event> {
 public:
  enum class Type : int {
    kWorkerCommon,
    kWorkerTimer,
    kWorkerUser,
    kEventConn,
  };

  Event() = default;
  virtual ~Event() {}

  /* 事件类型 */
  virtual Type GetType() { return Type::kWorkerUser; }

  /* 事件句柄 */
  virtual ev_handle_t GetHandle() const = 0;
};

}  // namespace myframe
