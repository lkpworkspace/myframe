#include "MyContext.h"

#include <assert.h>

#include "MyLog.h"
#include "MyModule.h"
#include "MyMsg.h"

MyContext::MyContext(std::shared_ptr<MyModule>& mod) :
    _mod(mod),
    m_handle(0),
    m_in_global(true),
    m_in_msg_list(false),
    m_session_id(0)
{
    SetInherits("MyNode");
    _mod->SetContext(this);
}

int MyContext::NewSession()
{
    int session = ++m_session_id;
    if (session <= 0) {
        m_session_id = 1;
        return 1;
    }
    return session;
}

int MyContext::SendMsg(std::shared_ptr<MyMsg>& msg)
{
    if(nullptr == msg) return -1;
    LOG(INFO) << "Service " << m_handle << " send message type: " << (int)msg->GetMsgType();
    if(msg->source == 0) { 
        msg->source = m_handle;
    }
    if((int)msg->GetCtrl() & (int)MyMsg::MyMsgCtrl::ALLOC_SESSION) {
        msg->session = NewSession();
    }
    _send.emplace_back(msg);
    return 0;
}

int MyContext::Init(const char* param)
{
    return _mod->Init(param);
}

void MyContext::CB(std::shared_ptr<MyMsg>& msg) {
    _mod->CB(msg);
}
