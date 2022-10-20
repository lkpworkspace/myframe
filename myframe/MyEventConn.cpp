/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <glog/logging.h>

#include "MyEventConn.h"
#include "MyCUtils.h"
#include "MyMsg.h"

namespace myframe {

MyEventConn::MyEventConn() {
    CreateSockPair();
}

MyEventConn::~MyEventConn() {
    CloseSockPair();
}

int MyEventConn::GetFd() {
    return _sockpair[1];
}

MyEventType MyEventConn::GetMyEventType() {
    return MyEventType::EVENT_CONN;
}

unsigned int MyEventConn::ListenEpollEventType() {
    return EPOLLIN;
}

void MyEventConn::RetEpollEventType(uint32_t ev) {
    ev = ev;
}

std::shared_ptr<MyMsg> MyEventConn::SendRequest(const std::string& dst, std::shared_ptr<MyMsg> req) {
    req->SetSrc(_ev_conn_name);
    req->SetDst(dst);
    _send.clear();
    _send.emplace_back(req);
    SendCmdToMain(MyWorkerCmd::IDLE);
    MyWorkerCmd cmd;
    RecvCmdFromMain(cmd);
    if (_recv.empty()) {
        return nullptr;
    }
    auto msg = _recv.front();
    _recv.clear();
    return msg;
}

int MyEventConn::SendCmdToWorker(const MyWorkerCmd& cmd) {
    char cmd_char = (char)cmd;
    return write(_sockpair[1], &cmd_char, 1);
}

int MyEventConn::RecvCmdFromWorker(MyWorkerCmd& cmd) {
    char cmd_char;
    int ret = read(_sockpair[1], &cmd_char, 1);
    cmd = (MyWorkerCmd)cmd_char;
    return ret;
}

int MyEventConn::RecvCmdFromMain(MyWorkerCmd& cmd, int timeout_ms) {
    if (timeout_ms < 0) {
        // block
        if (!my_is_block(_sockpair[0])) {
            my_set_nonblock(_sockpair[0], false);
        }
    } else if (timeout_ms == 0) {
        // nonblock
        if (my_is_block(_sockpair[0])) {
            my_set_nonblock(_sockpair[0], true);
        }
    } else {
        // timeout
        my_set_sock_recv_timeout(_sockpair[0], timeout_ms);
    }
    char cmd_char;
    int ret = read(_sockpair[0], &cmd_char, 1);
    cmd = (MyWorkerCmd)cmd_char;
    return ret;
}

int MyEventConn::SendCmdToMain(const MyWorkerCmd& cmd) {
    char cmd_char = (char)cmd;
    return write(_sockpair[0], &cmd_char, 1);
}

bool MyEventConn::CreateSockPair() {
    int res = -1;
    bool ret = true;

    res = socketpair(AF_UNIX,SOCK_DGRAM, 0, _sockpair);
    if(res == -1) {
        LOG(ERROR) << "Worker create sockpair failed";
        return false;
    }
    ret = my_set_nonblock(_sockpair[0], false);
    if(!ret) {
        LOG(ERROR) << "Worker set sockpair[0] block failed";
        return ret;
    }
    ret = my_set_nonblock(_sockpair[1], false);
    if(!ret) {
        LOG(ERROR) << "Worker set sockpair[1] block failed";
        return ret;
    }
    return ret;
}

void MyEventConn::CloseSockPair() {
    if(-1 == close(_sockpair[0])){
        LOG(ERROR) << "Worker close sockpair[0]: " << my_get_error();
    }
    if(-1 == close(_sockpair[1])){
        LOG(ERROR) << "Worker close sockpair[1]: " << my_get_error();
    }
}

void MyEventConn::SetEvConnName(const std::string& name) {
    _ev_conn_name = name;
}

std::string MyEventConn::GetEvConnName() {
    return _ev_conn_name;
}

} // namespace myframe