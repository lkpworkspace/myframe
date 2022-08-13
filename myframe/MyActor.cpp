#include "MyActor.h"
#include "MyContext.h"
#include "MyWorkerTimer.h"
#include "MyApp.h"

MyActor::MyActor() : 
    m_ctx(nullptr) {
}

MyActor::~MyActor()
{}

int MyActor::Send(const std::string& dst, std::shared_ptr<MyMsg> msg) {
    msg->SetSrc(GetActorName());
    msg->SetDst(dst);
    return m_ctx->SendMsg(msg);
}

uint32_t MyActor::GetHandle() {
    return m_ctx->GetHandle();
}

std::string MyActor::GetActorName() {
    return "actor." + m_actor_name + "." + m_instance_name;
}

int MyActor::Timeout(const std::string& timer_name, int expired) {
    return MyApp::Inst()->GetTimerWorker()->SetTimeout(GetActorName(), timer_name, expired);
}

void MyActor::SetContext(MyContext* c) {
    m_ctx = c;
}
