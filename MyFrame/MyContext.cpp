#include <assert.h>

#include "MyContext.h"
#include "MyModule.h"
#include "MyMsg.h"
#include "MyLog.h"
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

void MyContext::FilterArgs(int type, int* session, void** data, size_t* sz)
{
    int needcopy = !(type & MY_PTYPE_TAG_DONTCOPY);
    int allocsession = type & MY_PTYPE_TAG_ALLOCSESSION;
    type &= 0xff;

    if (allocsession) {
        assert(*session == 0);
        *session = NewSession();
    }

    if (needcopy && *data) {
        char * msg = (char*)malloc(*sz+1);
        memcpy(msg, *data, *sz);
        msg[*sz] = '\0';
        *data = msg;
    }

    *sz |= (size_t)type << MY_MESSAGE_TYPE_SHIFT;
}

int MyContext::SendMsg(uint32_t source,
                       uint32_t destination,
                       int type,
                       int session,
                       void* data,
                       size_t sz)
{
    if ((sz & MY_MESSAGE_TYPE_MASK) != sz) {
        MYLOG(MYLL_ERROR, ("The message to %x is too large", destination));
        if (type & MY_PTYPE_TAG_DONTCOPY) {
            free(data);
        }
        return -1;
    }
    FilterArgs(type, &session, (void**)&data, &sz);
    // 此处应该建立一个消息池供存取
    // 暂时使用new/delete的方法
    // 之后再进行修改
    // TODO...
    MyMsg* m = new MyMsg();
    if(source == 0)
        m->source = m_handle;
    else
        m->source = source;

    m->destination = destination;
    m->session = session;
    m->data = data;
    m->sz = sz;
    m_send.AddTail(static_cast<MyNode*>(m));
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
    int reserve_msg;
    int type = msg->sz >> MY_MESSAGE_TYPE_SHIFT;
    size_t sz = msg->sz & MY_MESSAGE_TYPE_MASK;
    if(m_cb){
        reserve_msg = m_cb(this, m_ud, type, msg->session, msg->source, msg->data, sz);
        if(!reserve_msg)
            free(msg->data);
    }else{
        MYLOG(MYLL_WARN, ("context %u callback is null\n", m_handle));
        return;
    }
    // 在该处释放MyMsg对象
    // 与SendMsg中的申请对应
    // TODO...
    delete msg;
}
