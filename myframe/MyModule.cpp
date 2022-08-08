#include "MyModule.h"
#include "MyContext.h"
#include "MyTimerWorker.h"
#include "MyApp.h"

MyModule::MyModule() : 
    m_ctx(nullptr) {
}

MyModule::~MyModule()
{}

int MyModule::Send(std::shared_ptr<MyMsg> msg) {
    return m_ctx->SendMsg(msg);
}

uint32_t MyModule::GetHandle() {
    return m_ctx->GetHandle();
}

std::string MyModule::GetServiceName() {
    return m_service_name + "." + m_instance_name;
}

int MyModule::Timeout(int time, int session) {
    return MyApp::Inst()->GetTimerWorker()->SetTimeout(GetHandle(), time, session);
}

void MyModule::SetContext(MyContext* c) {
    m_ctx = c;
}