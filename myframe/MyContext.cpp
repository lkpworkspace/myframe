#include <assert.h>

#include <sstream>

#include "MyContext.h"
#include "MyLog.h"
#include "MyActor.h"
#include "MyMsg.h"
#include "MyApp.h"

namespace myframe {

MyContext::MyContext(std::shared_ptr<MyApp> app, std::shared_ptr<MyActor> mod) :
    _app(app),
    _mod(mod),
    _in_worker(false),
    _in_wait_que(false) {
}

std::shared_ptr<MyApp> MyContext::GetApp() { 
    return _app.lock(); 
}

int MyContext::SendMsg(std::shared_ptr<MyMsg>& msg) {
    if(nullptr == msg) return -1;
    _send.emplace_back(msg);
    return 0;
}

int MyContext::Init(const char* param) {
    _mod->SetContext(shared_from_this());
    return _mod->Init(param);
}

void MyContext::Proc(const std::shared_ptr<const MyMsg>& msg) {
    _mod->Proc(msg);
}

std::string MyContext::Print() {
    std::stringstream ss;
    ss << "context " << _mod->GetActorName()
       << ", in worker: " << _in_worker
       << ", in wait queue: " << _in_wait_que;
    return ss.str();
}

} // namespace myframe