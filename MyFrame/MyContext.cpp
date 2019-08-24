#include "MyContext.h"

#include <assert.h>

#include <boost/log/trivial.hpp>

#include "MyModule.h"
#include "MyMsg.h"
#include "MyFrame.h"

MyContext::MyContext(MyModule* mod) :
    m_handle(0),
    m_mod(mod),
    m_in_global(true),
    m_session_id(0),
    m_cb(nullptr),
    m_ud(nullptr)
{}

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
    BOOST_LOG_TRIVIAL(debug) << "Service " << m_handle << " send message type: " << (int)msg->GetMsgType();
    if(msg->source == 0) msg->source = m_handle;
    if((int)msg->GetCtrl() & (int)MyMsg::MyMsgCtrl::ALLOC_SESSION) msg->session = NewSession();
    m_send.AddTail(static_cast<MyNode*>(msg));
    return 0;
}

int MyContext::Init(const char* param)
{
    return m_mod->Init(this, param);
}

void MyContext::SetCB(my_cb cb, void* ud)
{
    m_cb = cb;
    m_ud = ud;
}

void MyContext::CB(MyMsg* msg)
{
    if(m_cb){
        m_cb(this, msg, m_ud);
    }else{
        BOOST_LOG_TRIVIAL(warning) << "Context " << m_handle << " callback is null";
        return;
    }
    delete msg;
}
