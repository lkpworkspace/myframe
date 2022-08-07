#include "MyModule.h"
#include "MyApp.h"
#include "MyContext.h"
#include "MyTimerTask.h"
#include "MySock.h"
#include "MySocksMgr.h"

MyModule::MyModule() : 
    m_ctx(nullptr){
}

MyModule::~MyModule()
{}

int MyModule::Send(MyMsg* msg)
{
    return m_ctx->SendMsg(msg);
}

uint32_t MyModule::GetHandle()
{
    return m_ctx->GetHandle();
}

std::string MyModule::GetServiceName()
{
    return m_service_name + "." + m_instance_name;
}

void MyModule::SetRunInOneThread(bool b)
{
    m_ctx->SetRunInOneThread(b);
}

int MyModule::Timeout(int time, int session)
{
    return MyApp::Inst()->GetTimerTask()->SetTimeout(GetHandle(), time, session);
}


int MyModule::Listen(const char* addr, int port, int backlog)
{
    return MyApp::Inst()->GetSocksMgr()->Listen(m_ctx, addr, port, backlog);
}

int MyModule::SockSend(uint32_t id, const void* buf, int sz)
{
    return MyApp::Inst()->GetSocksMgr()->GetSock(id)->Send(buf,sz);
}

void MyModule::SetContext(MyContext* c)
{
    m_ctx = c;
}