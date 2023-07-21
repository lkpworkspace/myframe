/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/event_conn_manager.h"

#include <glog/logging.h>

#include "myframe/app.h"
#include "myframe/poller.h"
#include "myframe/event_conn.h"

namespace myframe {

EventConnManager::EventConnManager() {
  LOG(INFO) << "EventConnManager create";
}

EventConnManager::~EventConnManager() {
  LOG(INFO) << "EventConnManager deconstruct";
  std::lock_guard<std::mutex> g(mtx_);
  auto app = app_.lock();
  if (app != nullptr) {
    for (auto p : run_conn_) {
      app->poller_->Del(p.second);
    }
  }
  run_conn_.clear();
  run_conn_map_.clear();
  idle_conn_.clear();
}

bool EventConnManager::Init(std::shared_ptr<App> app, int sz) {
  app_ = app;
  for (int i = 0; i < sz; ++i) {
    std::lock_guard<std::mutex> g(mtx_);
    AddEventConn();
  }
  return true;
}

void EventConnManager::AddEventConn() {
  auto conn = std::make_shared<EventConn>();
  std::string name = "event.conn." + std::to_string(conn_sz_);
  conn->GetMailbox()->SetAddr(name);
  idle_conn_.emplace_back(conn);
  conn_sz_++;
}

std::shared_ptr<EventConn> EventConnManager::Get(int handle) {
  std::lock_guard<std::mutex> g(mtx_);
  if (run_conn_map_.find(handle) == run_conn_map_.end()) {
    DLOG(WARNING) << "can't find event conn, handle " << handle;
    return nullptr;
  }
  auto name = run_conn_map_[handle];
  if (run_conn_.find(name) == run_conn_.end()) {
    DLOG(WARNING) << "can't find event conn, name " << name;
    return nullptr;
  }
  return run_conn_[name];
}

std::shared_ptr<EventConn> EventConnManager::Get() {
  std::lock_guard<std::mutex> g(mtx_);
  // check has event conn
  if (idle_conn_.empty()) {
    AddEventConn();
  }
  auto app = app_.lock();
  if (app == nullptr) {
    LOG(ERROR) << "app is nullptr";
    return nullptr;
  }
  // remove from idle_conn
  auto conn = idle_conn_.front();
  idle_conn_.pop_front();
  // add to run_conn
  const auto& addr = conn->GetMailbox()->Addr();
  run_conn_[addr] = conn;
  run_conn_map_[conn->GetHandle()] = addr;
  // add to epoll
  app->poller_->Add(conn);
  return conn;
}

void EventConnManager::Release(std::shared_ptr<EventConn> ev) {
  std::lock_guard<std::mutex> g(mtx_);
  auto app = app_.lock();
  if (app == nullptr) {
    LOG(ERROR) << "app is nullptr";
    return;
  }
  // delete from epoll
  app->poller_->Del(ev);
  // remove from run_conn
  const auto& name = ev->GetMailbox()->Addr();
  run_conn_.erase(name);
  run_conn_map_.erase(ev->GetHandle());
  // add to idle_conn
  idle_conn_.emplace_back(ev);
}

// call by main frame
void EventConnManager::Notify(
  const std::string& name,
  std::shared_ptr<Msg> msg) {
  std::shared_ptr<EventConn> ev = nullptr;
  {
    std::lock_guard<std::mutex> g(mtx_);
    if (run_conn_.find(name) == run_conn_.end()) {
      LOG(WARNING) << "can't find " << name;
      return;
    }
    ev = run_conn_[name];
  }
  if (ev->GetConnType() == EventConn::Type::kSend) {
    return;
  }
  // push msg to event_conn
  ev->GetMailbox()->Recv(msg);
  // send cmd to event_conn
  auto cmd_channel = ev->GetCmdChannel();
  cmd_channel->SendToOwner(CmdChannel::Cmd::kIdle);
}

}  // namespace myframe
