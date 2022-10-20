/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include <glog/logging.h>

#include "MyEventConnManager.h"
#include "MyEventConn.h"
#include "MyApp.h"

namespace myframe {

MyEventConnManager::MyEventConnManager() {}
MyEventConnManager::~MyEventConnManager() {
    std::lock_guard<std::mutex> g(_mtx);
    auto app = _app.lock();
    if (app == nullptr) {
        LOG(ERROR) << "app is nullptr";
        return;
    }
    for (auto p : _run_conn) {
        app->DelEvent(p.second);
    }
    _run_conn.clear();
    _run_conn_map.clear();
    _idle_conn.clear();
}

bool MyEventConnManager::Init(std::shared_ptr<MyApp> app, int sz) {
    _app = app;
    for (int i = 0; i < sz; ++i) {
        std::lock_guard<std::mutex> g(_mtx);
        AddEventConn();
    }
    return true;
}

void MyEventConnManager::AddEventConn() {
    auto conn = std::make_shared<MyEventConn>();
    std::string name = "event.conn." + std::to_string(_conn_sz);
    conn->SetEvConnName(name);
    _idle_conn.emplace_back(conn);
    _conn_sz++;
}

std::shared_ptr<MyEventConn> MyEventConnManager::Get(int handle) {
    std::lock_guard<std::mutex> g(_mtx);
    if (_run_conn_map.find(handle) == _run_conn_map.end()) {
        DLOG(WARNING) << "can't find event conn, handle " << handle;
        return nullptr;
    }
    auto name = _run_conn_map[handle];
    if (_run_conn.find(name) == _run_conn.end()) {
        DLOG(WARNING) << "can't find event conn, name " << name;
        return nullptr;
    }
    return _run_conn[name];
}

std::shared_ptr<MyEventConn> MyEventConnManager::Get() {
    std::lock_guard<std::mutex> g(_mtx);
    // check has event conn
    if (_idle_conn.empty()) {
        AddEventConn();
    }
    auto app = _app.lock();
    if (app == nullptr) {
        LOG(ERROR) << "app is nullptr";
        return nullptr;
    }
    // remove from idle_conn
    auto conn = _idle_conn.front();
    _idle_conn.pop_front();
    // add to run_conn
    _run_conn[conn->GetEvConnName()] = conn;
    _run_conn_map[conn->GetFd()] = conn->GetEvConnName();
    // add to epoll
    app->AddEvent(conn);
    return conn;
}

void MyEventConnManager::Release(std::shared_ptr<MyEventConn> ev) {
    std::lock_guard<std::mutex> g(_mtx);
    auto app = _app.lock();
    if (app == nullptr) {
        LOG(ERROR) << "app is nullptr";
        return;
    }
    // delete from epoll
    app->DelEvent(ev);
    // remove from run_conn
    auto name = ev->GetEvConnName();
    _run_conn.erase(name);
    _run_conn_map.erase(ev->GetFd());
    // add to idle_conn
    _idle_conn.emplace_back(ev);
}

// call by main frame
void MyEventConnManager::Notify(const std::string& name, std::shared_ptr<MyMsg> msg) {
    std::shared_ptr<MyEventConn> ev = nullptr;
    {
        std::lock_guard<std::mutex> g(_mtx);
        if (_run_conn.find(name) == _run_conn.end()) {
            LOG(WARNING) << "can't find " << name;
            return;
        }
        ev = _run_conn[name];
    }
    // push msg to event_conn
    ev->_recv.emplace_back(msg);
    // send cmd to event_conn
    ev->SendCmdToWorker(MyWorkerCmd::RUN);
}

} // namespace myframe