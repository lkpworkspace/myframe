/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <memory>

namespace myframe {

enum class EventType : int {
  WORKER_COMMON,
  WORKER_TIMER,
  WORKER_USER,
  EVENT_CONN,
};

class Event : public std::enable_shared_from_this<Event> {
 public:
  Event() {}
  virtual ~Event() {}

  /* 事件类型 */
  virtual EventType GetType() { return EventType::WORKER_USER; }

  /* 获得当前事件的文件描述符 */
  virtual int GetFd() = 0;

  /**
   * 监听的是文件描述符的写事件还是读事件
   * 一般是读或写事件(EPOLLIN/EPOLLOUT)
   */
  virtual unsigned int ListenEpollEventType() = 0;

  /* 获得的epoll事件类型(call by App) */
  virtual void RetEpollEventType(uint32_t ev) = 0;
};

}  // namespace myframe
