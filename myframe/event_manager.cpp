/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/event_manager.h"

#include <glog/logging.h>

#include "myframe/worker.h"
#include "myframe/event_conn.h"

namespace myframe {

EventManager::EventManager() {
  LOG(INFO) << "EventManager create";
}

EventManager::~EventManager() {
  LOG(INFO) << "EventManager deconstruct";
  std::unique_lock<std::shared_mutex> lk(rw_);
  name_handle_map_.clear();
  evs_.clear();
}

ev_handle_t EventManager::ToHandle(const std::string& name) {
  std::shared_lock<std::shared_mutex> lk(rw_);
  if (name_handle_map_.find(name) == name_handle_map_.end()) {
    return Event::DEFAULT_EV_HANDLE;
  }
  return name_handle_map_[name];
}

bool EventManager::Has(const std::string& name) {
  std::shared_lock<std::shared_mutex> lk(rw_);
  return name_handle_map_.find(name) != name_handle_map_.end();
}

std::vector<std::shared_ptr<Event>> EventManager::Get(
    const std::vector<Event::Type>& type_list) {
  std::shared_lock<std::shared_mutex> lk(rw_);
  std::vector<std::shared_ptr<Event>> tmp_evs;
  for (auto it = evs_.begin(); it != evs_.end(); ++it) {
    for (size_t i = 0; i < type_list.size(); ++i) {
      if (type_list[i] == it->second->GetType()) {
        tmp_evs.push_back(it->second);
        break;
      }
    }
  }
  return tmp_evs;
}

bool EventManager::Add(std::shared_ptr<Event> ev) {
  auto handle = ev->GetHandle();
  std::unique_lock<std::shared_mutex> lk(rw_);
  if (evs_.find(handle) != evs_.end()) {
    LOG(ERROR) << " add handle " << handle << " has exist";
    return false;
  }
  evs_[handle] = ev;
  name_handle_map_[ev->GetName()] = handle;
  return true;
}

bool EventManager::Del(std::shared_ptr<Event> ev) {
  auto handle = ev->GetHandle();
  std::unique_lock<std::shared_mutex> lk(rw_);
  if (evs_.find(handle) == evs_.end()) {
    LOG(ERROR) << " del handle " << handle << " has exist";
    return false;
  }
  evs_.erase(handle);
  name_handle_map_.erase(ev->GetName());
  return true;
}

}  // namespace myframe
