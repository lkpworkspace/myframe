#include "MyActor.h"
#include "MyContext.h"
#include "MyWorkerTimer.h"
#include "MyApp.h"

MyActor::MyActor() : 
    m_ctx(nullptr) {
}

MyActor::~MyActor()
{}

int MyActor::Send(std::shared_ptr<MyMsg> msg) {
    return m_ctx->SendMsg(msg);
}

uint32_t MyActor::GetHandle() {
    return m_ctx->GetHandle();
}

std::string MyActor::GetServiceName() {
    return m_service_name + "." + m_instance_name;
}

int MyActor::Timeout(int time, int session) {
    return MyApp::Inst()->GetTimerWorker()->SetTimeout(GetHandle(), time, session);
}

void MyActor::SetContext(MyContext* c) {
    m_ctx = c;
}