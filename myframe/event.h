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
  enum class IOType : int {
    kNone,
    kIn,
    kOut,
  };

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

  /* 获得当前事件的文件描述符 */
  virtual ev_handle_t GetHandle() const = 0;

  /* 监听的是文件描述符的写事件还是读事件 */
  virtual IOType ListenIOType() { return IOType::kIn; }

  /* 返回的监听事件类型 */
  virtual void RetListenIOType(const IOType&) {}
};

}  // namespace myframe
