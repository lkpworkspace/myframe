/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "MyWorkerCommon.h"
#include "MyLog.h"
#include "MyContext.h"
#include "MyMsg.h"

namespace myframe {

MyWorkerCommon::MyWorkerCommon()
{}

MyWorkerCommon::~MyWorkerCommon() {
}

void MyWorkerCommon::Idle() {
    if(!_context.expired()) {
        _context.reset();
    }
}

void MyWorkerCommon::Run() {
    DispatchMsg();
    Work();
}

void MyWorkerCommon::OnInit() {
    MyWorker::OnInit();
    LOG(INFO) << "Worker " << GetWorkerName() << " init";
}

void MyWorkerCommon::OnExit() {
    MyWorker::OnExit();
    LOG(INFO) << "Worker " << GetWorkerName() << " exit";
}

int MyWorkerCommon::Work() {
    auto ctx = (_context.expired() ? nullptr : _context.lock());
    if (ctx == nullptr) {
        LOG(ERROR) << "context is nullptr";
        return -1;
    }
    while (RecvMsgListSize() > 0) {
        ctx->Proc(GetRecvMsg());
    }
    
    return 0;
}

} // namespace myframe