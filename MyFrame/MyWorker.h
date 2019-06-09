#ifndef __MYWORKER_H__
#define __MYWORKER_H__

#include "MyCommon.h"
#include "MyThread.h"

class MyContext;
class MyWorker : public MyThread
{
    friend class MyApp;
public:
    MyWorker();
    ~MyWorker();

    /**
     * override MyThread virtual method
     */
    virtual void Run() override;
    virtual void OnInit() override;
    virtual void OnExit() override;

    /**
     * override MyEvent virtual method
     */
    virtual int GetEventType() override
    { return EV_WORKER; }
    virtual int GetFd() override;
    virtual unsigned int GetEpollEventType() override;
    virtual MyList* CB(MyEvent*, int*) override
    { return nullptr; }
    virtual void SetEpollEvents(uint32_t ev) override
    { ev = ev; }

    // 主线程调用该函数与工作线程通信
    int SendCmd(const char* cmd, size_t len);
    int RecvCmd(char* cmd, size_t len);

    void SetContext(MyContext* context){ m_context = context; }

private:
    /* 等待主线程唤醒工作 */
    int Wait();
    /* 工作线程消息处理 */
    int Work();
    /* 工作线程进入空闲链表之前进行的操作 */
    void Idle();
    /* 创建与主线程通信的socket */
    bool CreateSockPair();
    void CloseSockPair();

    /* idx: 0 used by MyWorker, 1 used by MyApp */
    int               m_sockpair[2];

    MyList            m_send;              // 发送消息队列(针对没有服务的消息,缓存到该队列)
    MyList            m_que;               // work queue
    MyContext*        m_context;           // 当前执行服务的指针
};

#endif
