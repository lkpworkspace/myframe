#pragma once
#include <memory>
#include <list>

#include "MyCommon.h"
#include "MyList.h"

class MyApp;
class MyMsg;
class MyActor;
class MyWorkerCommon;
class MyContext : public MyNode
{
    friend class MyContextManager;
    friend class MyWorkerCommon;
    friend class MyApp;
public:
    MyContext(std::shared_ptr<MyApp> app, std::shared_ptr<MyActor> mod);
    virtual ~MyContext(){}

    int Init(const char *param);

    /* actor将消息添加至发送队列中 */
    int SendMsg(std::shared_ptr<MyMsg>& msg);

    /* 工作线程调用回调让actor去处理消息 */
    void CB(const std::shared_ptr<const MyMsg>& msg);

    /* 主线程发送消息给该actor */
    void PushMsg(std::shared_ptr<MyMsg>& msg){ _recv.emplace_back(msg); }

    /* 主线程获得该actor待处理消息链表 */
    std::list<std::shared_ptr<MyMsg>>& GetRecvMsgList(){ return _recv; }
    
    /* 主线程获得该actor发送消息链表 */
    std::list<std::shared_ptr<MyMsg>>& GetDispatchMsgList(){ return _send; }

    uint32_t GetHandle() { return _handle; }
    void SetHandle(uint32_t handle) { _handle = handle; }

    void SetRunFlag() { _in_worker = true; }

    void SetWaitFlag() { _in_worker = false; }

    bool IsRuning() { return _in_worker; }

    void SetOutOfRunQueueFlag() { _in_run_que = false; }

    void SetInRunQueueFlag() { _in_run_que = true; }

    bool IsInRunQueue() { return _in_run_que; }

    std::shared_ptr<MyActor> GetModule() { return _mod; }
    std::shared_ptr<MyApp> GetApp();

    std::string Print();
private:
    /* actor句柄 */
    uint32_t            _handle;
    /* 主线程分发给actor的消息链表，主线程操作该链表 */
    std::list<std::shared_ptr<MyMsg>> _recv;
    /* actor发送给别的actor的消息链表，工作线程处理该actor时可以操作该链表, 空闲时主线程操作该链表 */
    std::list<std::shared_ptr<MyMsg>> _send;
    /* 该actor的是否在工作线程的标志 */
    bool                _in_worker;
    /* actor是否在消息队列中 */
    bool                _in_run_que;
    std::shared_ptr<MyActor> _mod;
    std::weak_ptr<MyApp> _app;
};
