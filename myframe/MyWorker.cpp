/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "MyCommon.h"
#include "MyWorker.h"
#include "MyLog.h"
#include "MyCUtils.h"

namespace myframe {

MyWorker::MyWorker() :
    _posix_thread_id(-1),
    _runing(false) {
    CreateSockPair();
}

MyWorker::~MyWorker() {
    CloseSockPair();
}

const std::string MyWorker::GetWorkerName() const {
    return "worker." + _worker_name + "." + _inst_name;
}

void MyWorker::Start() {
    int res = 0;
    if(_runing == false) {
        _runing = true;
        res = pthread_create(&_posix_thread_id, NULL, &MyWorker::ListenThread, this);
        if(res != 0) {
            _runing = false;
            LOG(ERROR) << "pthread create failed";
            return;
        }
        res = pthread_detach(_posix_thread_id);
        if(res != 0) {
            _runing = false;
            LOG(ERROR) << "pthread detach failed";
            return;
        }
    }
}

void MyWorker::Stop() {
    _runing = false;
    MyWorkerCmd cmd = MyWorkerCmd::QUIT;
    SendCmdToMain(cmd);
}

void* MyWorker::ListenThread(void* obj) {
    MyWorker* t = static_cast<MyWorker*>(obj);
    t->OnInit();
    while (t->_runing)
		t->Run();
	t->OnExit();
    return nullptr;
}

int MyWorker::SendCmdToMain(const MyWorkerCmd& cmd) {
    char cmd_char = (char)cmd;
    return write(_sockpair[0], &cmd_char, 1);
}

int MyWorker::SendCmdToWorker(const MyWorkerCmd& cmd) {
    char cmd_char = (char)cmd;
    return write(_sockpair[1], &cmd_char, 1);
}

int MyWorker::RecvCmdFromMain(MyWorkerCmd& cmd, int timeout_ms) {
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

int MyWorker::RecvCmdFromWorker(MyWorkerCmd& cmd) {
    char cmd_char;
    int ret = read(_sockpair[1], &cmd_char, 1);
    cmd = (MyWorkerCmd)cmd_char;
    return ret;
}

void MyWorker::SendMsg(const std::string& dst, std::shared_ptr<MyMsg> msg) {
    msg->SetSrc(GetWorkerName());
    msg->SetDst(dst);
    _send.emplace_back(msg);
}

void MyWorker::PushSendMsgList(std::list<std::shared_ptr<MyMsg>>& msg_list) {
    MyListAppend(_send, msg_list);
}

int MyWorker::DispatchMsg() {
    MyWorkerCmd cmd = MyWorkerCmd::IDLE;
    SendCmdToMain(cmd);
    return RecvCmdFromMain(cmd);
}

int MyWorker::DispatchAndWaitMsg() {
    MyWorkerCmd cmd = MyWorkerCmd::WAIT_FOR_MSG;
    SendCmdToMain(cmd);
    return RecvCmdFromMain(cmd);
}

const std::shared_ptr<const MyMsg> MyWorker::GetRecvMsg() { 
    if (_que.empty()) {
        return nullptr;
    }
    auto msg = _que.front();
    _que.pop_front();
    return msg;
}

bool MyWorker::CreateSockPair() {
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

void MyWorker::CloseSockPair() {
    if(-1 == close(_sockpair[0])){
        LOG(ERROR) << "Worker close sockpair[0]: " << my_get_error();
    }
    if(-1 == close(_sockpair[1])){
        LOG(ERROR) << "Worker close sockpair[1]: " << my_get_error();
    }
}

} // namespace myframe