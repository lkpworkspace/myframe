/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#pragma once
#include <memory>
#include <list>

#include "MyEvent.h"
#include "MyWorker.h"

namespace myframe {

class MyMsg;
class MyEventConnManager;
class MyEventConn final : public MyEvent {
    friend class MyApp;
    friend class MyEventConnManager;
public:
    MyEventConn();
    virtual ~MyEventConn();

    int GetFd() override;
    MyEventType GetMyEventType() override;
    unsigned int ListenEpollEventType() override;
    void RetEpollEventType(uint32_t ev) override;

    std::shared_ptr<MyMsg> SendRequest(const std::string& dst, std::shared_ptr<MyMsg> req);

private:
    std::string GetEvConnName();
    void SetEvConnName(const std::string& name);

    int SendCmdToWorker(const MyWorkerCmd& cmd);
    int RecvCmdFromWorker(MyWorkerCmd& cmd);
    
    int RecvCmdFromMain(MyWorkerCmd& cmd, int timeout_ms = -1);
    int SendCmdToMain(const MyWorkerCmd& cmd);

    bool CreateSockPair();
    void CloseSockPair();
    int _sockpair[2];
   
    /// 接收消息队列
    std::list<std::shared_ptr<MyMsg>> _recv;
    /// 发送消息队列
    std::list<std::shared_ptr<MyMsg>> _send;

    std::string _ev_conn_name{""};
};

} // namespace myframe