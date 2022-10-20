/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <mutex>
#include <memory>
#include <list>
#include <unordered_map>

namespace myframe {

class MyApp;
class MyMsg;
class MyEventConn;
class MyEventConnManager final {
    friend class MyApp;
public:
    MyEventConnManager();
    virtual ~MyEventConnManager();

    bool Init(std::shared_ptr<MyApp> app, int sz = 2);

    std::shared_ptr<MyEventConn> Get();

    std::shared_ptr<MyEventConn> Get(int handle);

    void Release(std::shared_ptr<MyEventConn>);

private:
    void AddEventConn();
    void Notify(const std::string& name, std::shared_ptr<MyMsg> msg);

    int _conn_sz{0};
    std::mutex _mtx;
    std::unordered_map<int, std::string> _run_conn_map;
    std::unordered_map<std::string, std::shared_ptr<MyEventConn>> _run_conn;
    std::list<std::shared_ptr<MyEventConn>> _idle_conn;

    std::weak_ptr<MyApp> _app;
};

} // namespace myframe