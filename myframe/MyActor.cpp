#include "MyActor.h"
#include "MyContext.h"
#include "MyWorkerTimer.h"
#include "MyApp.h"

MyActor::MyActor() : 
    m_ctx(nullptr) {
}

MyActor::~MyActor()
{}

void MyActor::SetModName(const std::string& name) {
    if (_mod_name == "class") {
        _is_from_lib = false;
    } else {
        _is_from_lib = true;
    }
    _mod_name = name;
}

int MyActor::Send(const std::string& dst, std::shared_ptr<MyMsg> msg) {
    msg->SetSrc(GetActorName());
    msg->SetDst(dst);
    return m_ctx->SendMsg(msg);
}

uint32_t MyActor::GetHandle() const {
    return m_ctx->GetHandle();
}

const std::string MyActor::GetActorName() const {
    return "actor." + _actor_name + "." + _instance_name;
}

int MyActor::Timeout(const std::string& timer_name, int expired) {
    return m_ctx->GetApp()->GetTimerWorker()->SetTimeout(GetActorName(), timer_name, expired);
}

void MyActor::SetContext(MyContext* c) {
    m_ctx = c;
}
