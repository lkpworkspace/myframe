#ifndef __MYCONTEXT_H__
#define __MYCONTEXT_H__

#include "MyCommon.h"
#include "MyList.h"
#include "MyModule.h"
#include "MyFrame.h"

class MyMsg;
class MyModule;
class MyWorker;
class MySocksMgr;
class MyContext : public MyObj
{
    friend class MyHandleMgr;
    friend class MySocksMgr;
    friend class MyWorker;
    friend class MyApp;
public:
    MyContext(MyModule* mod);
    virtual ~MyContext(){}

    int Init(const char *param);

    /* 设置回调函数 */
    void SetCB(my_cb cb, void* ud);
    /* 服务将消息添加至发送队列中 */
    int SendMsg( uint32_t source,
                 uint32_t destination,
                 int type,
                 int session,
                 void* msg,
                 size_t sz);

    /* 工作线程调用回调让服务去处理消息 */
    void CB(MyMsg* msg);

    /* 主线程发送消息给该服务 */
    void PushMsg(MyNode* msg){ m_recv.AddTail(msg); }
    /* 主线程获得该服务待处理消息链表 */
    MyList* GetRecvMsgList(){ return &m_recv; }
    /* 主线程获得该服务发送消息链表 */
    MyList* GetDispatchMsgList(){ return &m_send; }

    uint32_t GetHandle() { return m_handle; }
private:
    void FilterArgs(int type, int* session, void** data, size_t* sz);
    int NewSession();

    /* 服务句柄 */
    uint32_t            m_handle;
    /* 主线程分发给服务的消息链表，主线程操作该链表 */
    MyList              m_recv;
    /* 服务发送给别的服务的消息链表，工作线程处理该服务时可以操作该链表, 空闲时主线程操作该链表 */
    MyList              m_send;
    MyModule*           m_mod;
    /* 该服务的是否在工作线程的标志 */
    bool                m_in_global;
    /* 服务分配的session ID */
    int                 m_session_id;
    my_cb               m_cb;
    void*               m_ud;
};

#endif
