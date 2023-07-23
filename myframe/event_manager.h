/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#pragma once
#include <memory>
#include <string>
#include <shared_mutex>
#include <vector>
#include <unordered_map>

#include "myframe/macros.h"
#include "myframe/event.h"

namespace myframe {

class EventConnManager;
class WorkerContextManager;
class EventManager final {
  friend class EventConnManager;
  friend class WorkerContextManager;

 public:
  EventManager();
  virtual ~EventManager();

  template<typename T>
  std::shared_ptr<T> Get(ev_handle_t h) {
    std::shared_lock<std::shared_mutex> lk(rw_);
    if (evs_.find(h) == evs_.end()) {
      return nullptr;
    }
    return std::dynamic_pointer_cast<T>(evs_[h]);
  }

  template<typename T>
  std::shared_ptr<T> Get(const std::string& name) {
    std::shared_lock<std::shared_mutex> lk(rw_);
    if (name_handle_map_.find(name) == name_handle_map_.end()) {
      return nullptr;
    }
    lk.unlock();
    return Get<T>(name_handle_map_[name]);
  }

  bool Has(const std::string& name);

  std::vector<std::shared_ptr<Event>> Get(
    const std::vector<Event::Type>&);

  ev_handle_t ToHandle(const std::string&);

 private:
  bool Add(std::shared_ptr<Event>);
  bool Del(std::shared_ptr<Event>);

  std::shared_mutex rw_;
  std::unordered_map<std::string, ev_handle_t> name_handle_map_;
  std::unordered_map<ev_handle_t, std::shared_ptr<Event>> evs_;

  DISALLOW_COPY_AND_ASSIGN(EventManager)
};

}  // namespace myframe
