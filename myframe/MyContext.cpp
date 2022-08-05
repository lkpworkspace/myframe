#include "MyContext.h"

#include <assert.h>

#include "MyLog.h"
#include "MyModule.h"
#include "MyMsg.h"

MyContext::MyContext(std::shared_ptr<MyModule>& mod) :
    m_handle(0),
    m_mod(mod),
    m_in_global(true),
    m_in_msg_list(false),
    m_run_in_one_thread(false),
    m_session_id(0)
{
    SetInherits("MyNode");
    m_mod->SetContext(this);
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

int MyContext::SendMsg(MyMsg* msg)
{
    if(nullptr == msg) return -1;
    LOG(INFO) << "Service " << m_handle << " send message type: " << (int)msg->GetMsgType();
    if(msg->source == 0) msg->source = m_handle;
    if((int)msg->GetCtrl() & (int)MyMsg::MyMsgCtrl::ALLOC_SESSION) msg->session = NewSession();
    m_send.AddTail(static_cast<MyNode*>(msg));
    return 0;
}

int MyContext::Init(const char* param)
{
    return m_mod->Init(param);
}

void MyContext::CB(MyMsg* msg)
{
    if(1 == m_mod->CB(msg)){
        delete msg;
    }
}
