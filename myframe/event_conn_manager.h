/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <string>

#include "myframe/macros.h"
#include "myframe/event.h"

namespace myframe {

class Msg;
class Poller;
class EventManager;
class EventConn;
class EventConnManager final {
 public:
  EventConnManager(
    std::shared_ptr<EventManager>,
    std::shared_ptr<Poller>);
  virtual ~EventConnManager();

  bool Init(int sz = 2);

  std::shared_ptr<EventConn> Alloc();

  void Release(std::shared_ptr<EventConn>);

  void Notify(ev_handle_t, std::shared_ptr<Msg> msg);

 private:
  void AddEventConn();

  int conn_sz_{0};
  std::mutex mtx_;
  std::list<std::shared_ptr<EventConn>> idle_conn_;
  std::shared_ptr<EventManager> ev_mgr_;
  std::shared_ptr<Poller> poller_;

  DISALLOW_COPY_AND_ASSIGN(EventConnManager)
};

}  // namespace myframe
