#ifndef __MYCONTEXT_H__
#define __MYCONTEXT_H__
#include <memory>
#include <list>

#include "MyCommon.h"
#include "MyList.h"

class MyMsg;
class MyModule;
class MyWorker;
class MyContext : public MyNode
{
    friend class MyHandleMgr;
    friend class MyWorker;
    friend class MyApp;
public:
    MyContext(std::shared_ptr<MyModule>& mod);
    virtual ~MyContext(){}

    int Init(const char *param);

    /* 服务将消息添加至发送队列中 */
    int SendMsg(std::shared_ptr<MyMsg>& msg);

    /* 工作线程调用回调让服务去处理消息 */
    void CB(std::shared_ptr<MyMsg>& msg);

    /* 主线程发送消息给该服务 */
    void PushMsg(std::shared_ptr<MyMsg>& msg){ _recv.emplace_back(msg); }

    /* 主线程获得该服务待处理消息链表 */
    std::list<std::shared_ptr<MyMsg>>& GetRecvMsgList(){ return _recv; }
    
    /* 主线程获得该服务发送消息链表 */
    std::list<std::shared_ptr<MyMsg>>& GetDispatchMsgList(){ return _send; }

    uint32_t GetHandle() { return m_handle; }

    std::shared_ptr<MyModule> GetModule() { return _mod; }

private:
    void FilterArgs(int type, int* session, void** data, size_t* sz);
    int NewSession();

    /* 服务句柄 */
    uint32_t            m_handle;
    /* 主线程分发给服务的消息链表，主线程操作该链表 */
    std::list<std::shared_ptr<MyMsg>> _recv;
    /* 服务发送给别的服务的消息链表，工作线程处理该服务时可以操作该链表, 空闲时主线程操作该链表 */
    std::list<std::shared_ptr<MyMsg>> _send;
    /* 该服务的是否在工作线程的标志 */
    bool                m_in_global;
    /* 服务是否在消息队列中 */
    bool                m_in_msg_list;
    /* 服务分配的session ID */
    int                 m_session_id;
    std::shared_ptr<MyModule> _mod;

};

#endif
