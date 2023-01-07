/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <string>

namespace myframe {

class App;
class Msg;
class EventConn;
class EventConnManager final {
  friend class App;

 public:
  EventConnManager();
  virtual ~EventConnManager();

  bool Init(std::shared_ptr<App> app, int sz = 2);

  std::shared_ptr<EventConn> Get();

  std::shared_ptr<EventConn> Get(int handle);

  void Release(std::shared_ptr<EventConn>);

 private:
  void AddEventConn();
  void Notify(const std::string& name, std::shared_ptr<Msg> msg);

  int conn_sz_{0};
  std::mutex mtx_;
  std::unordered_map<int, std::string> run_conn_map_;
  std::unordered_map<std::string, std::shared_ptr<EventConn>> run_conn_;
  std::list<std::shared_ptr<EventConn>> idle_conn_;

  std::weak_ptr<App> app_;
};

}  // namespace myframe
