#include <assert.h>

#include <sstream>

#include "MyContext.h"
#include "MyLog.h"
#include "MyActor.h"
#include "MyMsg.h"

MyContext::MyContext(std::shared_ptr<MyActor>& mod) :
    _mod(mod),
    _handle(0),
    _in_worker(false),
    _in_run_que(false) {
    _mod->SetContext(this);
}

int MyContext::SendMsg(std::shared_ptr<MyMsg>& msg) {
    if(nullptr == msg) return -1;
    _send.emplace_back(msg);
    return 0;
}

int MyContext::Init(const char* param) {
    return _mod->Init(param);
}

void MyContext::CB(const std::shared_ptr<const MyMsg>& msg) {
    _mod->CB(msg);
}

std::string MyContext::Print() {
    std::stringstream ss;
    ss << "context " << _mod->GetActorName()
       << ", in worker: " << _in_worker
       << ", in run queue: " << _in_run_que;
    return ss.str();
}