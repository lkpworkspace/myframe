#include "MyModule.h"
#include "MyApp.h"
#include "MyContext.h"
#include "MyTimerTask.h"
#include "MySock.h"
#include "MySocksMgr.h"

MyModule::MyModule() :
    m_ctx(nullptr)
{}

MyModule::MyModule(std::string mod_name, std::string service_name) : 
    m_mod_name(mod_name),
    m_service_name(service_name),
    m_ctx(nullptr)
{

}

MyModule::~MyModule()
{

}

int MyModule::Send(MyMsg* msg)
{
    return m_ctx->SendMsg(msg);
}

uint32_t MyModule::GetHandle()
{
    return m_ctx->GetHandle();
}

uint32_t MyModule::GetHandle(std::string service_name)
{
    MyContext* ctx = nullptr;
    ctx = MyApp::Inst()->GetContext(service_name);
    return (ctx == nullptr) ? -1 : ctx->GetHandle();
}

std::string MyModule::GetServiceName()
{
    return m_service_name;
}

std::string MyModule::GetServiceName(uint32_t handle)
{
    MyContext* ctx = nullptr;
    ctx = MyApp::Inst()->GetContext(handle);
    return (ctx == nullptr) ? "" : ctx->GetModule()->GetServiceName();
}

uint32_t MyModule::CreateService(MyModule* mod_inst, const char* params)
{
    uint32_t handle = 0x00;
    if(MyApp::Inst()->CreateContext(mod_inst, params)){
        handle = GetHandle(mod_inst->GetServiceName());
    }
    return handle;
}
uint32_t MyModule::CreateService(std::string mod_name, std::string service_name, const char* params)
{
    uint32_t handle = 0x00;
    if(MyApp::Inst()->CreateContext(mod_name.c_str(), service_name.c_str(), params)){
        handle = GetHandle(service_name);
    }
    return handle;
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