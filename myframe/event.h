/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <memory>

namespace myframe {

enum class EventIOType : int {
  kNone,
  kIn,
  kOut,
};

enum class EventType : int {
  kWorkerCommon,
  kWorkerTimer,
  kWorkerUser,
  kEventConn,
};

class Event : public std::enable_shared_from_this<Event> {
 public:
  Event() {}
  virtual ~Event() {}

  /* 事件类型 */
  virtual EventType GetType() { return EventType::kWorkerUser; }

  /* 获得当前事件的文件描述符 */
  virtual int GetFd() const = 0;

  /**
   * 监听的是文件描述符的写事件还是读事件
   */
  virtual EventIOType ListenIOType() { return EventIOType::kIn; }

  /* 返回的监听事件类型 */
  virtual void RetListenIOType(const EventIOType&) {}
};

}  // namespace myframe
