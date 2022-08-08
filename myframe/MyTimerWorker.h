#ifndef __MYTIMERWORKER_H__
#define __MYTIMERWORKER_H__
#include <mutex>
#include <memory>
#include <list>

#include "MyCommon.h"
#include "MyThread.h"
#include "MyMsg.h"

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)

#define MY_RESOLUTION_MS 10

class MyTimer : public MyNode
{
    friend class MyTimerMgr;
public:
    MyTimer(){}
    virtual ~MyTimer(){}

    uint32_t m_handle;
    uint32_t m_session;
    uint32_t m_expire;        // interval
    bool     m_run;
};

class MyTimerMgr
{
public:
    MyTimerMgr();
    virtual ~MyTimerMgr();

    int Timeout(uint32_t handle, int time, int session);

    std::list<std::shared_ptr<MyMsg>>& Updatetime();

private:
    void _AddTimerNode(MyTimer* node);
    void _Updatetime();
    void _Execute();
    void _MoveList(int level, int idx);
    void _Shift();
    void _Dispath(MyList *cur);

    MyList            m_tv1[TVR_SIZE];
    MyList            m_tv2[TVN_SIZE];
    MyList            m_tv3[TVN_SIZE];
    MyList            m_tv4[TVN_SIZE];
    MyList            m_tv5[TVN_SIZE];
    MyList*           m_tv[4];

    uint32_t          m_time;
    uint64_t          m_cur_point;

    std::list<std::shared_ptr<MyMsg>> m_timeout;
    std::mutex        m_mutex;
};

class MyTimerWorker : public MyThread
{
    friend class MyApp;
public:
    MyTimerWorker();
    virtual ~MyTimerWorker();

    int SetTimeout(uint32_t handle, int time, int session);

    /**
     * override MyThread virtual method
     */
    void Run() override;
    void OnInit() override;
    void OnExit() override;

    /**
     * override MyEvent virtual method
     */
    int GetEventType() override
    { return EV_TIMER; }
    int GetFd() override
    { return m_sockpair[1]; }
    unsigned int ListenEpollEventType() override
    { return EPOLLIN; }
    void RetEpollEventType(uint32_t ev) override
    { ev = ev; }

    // 主线程调用该函数与工作线程通信
    int SendCmd(const char* cmd, size_t len);
    int RecvCmd(char* cmd, size_t len);

private:
    int Wait();
    int Work();
    /* 创建与主线程通信的socket */
    bool CreateSockPair();
    void CloseSockPair();

    MyTimerMgr        m_timer_mgr;
    // timer定时通知消息队列
    std::list<std::shared_ptr<MyMsg>> _send;
    /* idx: 0 used by MyWorker, 1 used by MyApp */
    int               m_sockpair[2];
};

#endif
