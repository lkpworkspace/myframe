#include "MyContext.h"

#include <assert.h>

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
    LOG(INFO) << "actor \"" << _mod->GetActorName() << "\" send message type: " << msg->GetMsgType();
    _send.emplace_back(msg);
    return 0;
}

int MyContext::Init(const char* param) {
    return _mod->Init(param);
}

void MyContext::CB(std::shared_ptr<MyMsg>& msg) {
    _mod->CB(msg);
}
