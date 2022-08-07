#include "MyWorker.h"

#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "MyLog.h"
#include "MyCUtils.h"
#include "MyContext.h"
#include "MyMsg.h"
#include "MyApp.h"
#include "MyHandleMgr.h"

MyWorker::MyWorker() :
    _context(nullptr),
    _cmd(MyWorkerCmdType::IDLE) {
    CreateSockPair();
}

MyWorker::~MyWorker() {
    CloseSockPair();
}

void MyWorker::Idle() {
    if(_context){
        _context->m_in_global = true;
        _context = nullptr;
    }
}

void MyWorker::Run() {
    Wait();
    Work();
}

void MyWorker::OnInit() {
    MyThread::OnInit();
    LOG(INFO) << "Worker " << GetThreadId() << " init";
}

void MyWorker::OnExit() {
    MyThread::OnExit();
    LOG(INFO) << "Worker " << GetThreadId() << " exit";
}

int MyWorker::Work() {
    MyContext* ctx = _context;
    if (ctx == nullptr) {
        LOG(ERROR) << "context is nullptr";
        return -1;
    }

    for (auto msg : _que) {
        ctx->CB(msg);
        LOG(INFO) << "Worker: " << GetThreadId() << " get cmd: "
            << (char)_cmd;
    }
    _que.clear();
    return 0;
}

int MyWorker::GetFd() {
    return _sockpair[1];
}

unsigned int MyWorker::ListenEpollEventType() {
    return EPOLLIN;
}

int MyWorker::SendCmd(const char* cmd, size_t len) {
    return write(_sockpair[1], cmd, len);
}

int MyWorker::RecvCmd(char* cmd, size_t len) {
    return read(_sockpair[1], cmd, len);
}

int MyWorker::Wait() {
    // tell MyApp, add this worker to idle worker list
    char cmd = (char)_cmd; // idle
    write(_sockpair[0], &cmd, 1);

    // 等待主线程唤醒工作
    return read(_sockpair[0], &cmd, 1);
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
