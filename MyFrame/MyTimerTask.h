#ifndef __MYTIMERTASK_H__
#define __MYTIMERTASK_H__
#include <map>
#include <mutex>
#include "MyCommon.h"
#include "MyThread.h"

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

    MyList *Updatetime();

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

    MyList            m_timeout;
    std::mutex        m_mutex;
};

class MyTimerTask : public MyThread
{
    friend class MyApp;
public:
    MyTimerTask();
    virtual ~MyTimerTask();

    int SetTimeout(uint32_t handle, int time, int session);

    //void StartTimer(uint32_t id);
    //void StopTimer(uint32_t id);

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
    { return EV_TIMER; }
    virtual int GetFd() override
    { return m_sockpair[1]; }
    virtual unsigned int GetEpollEventType() override
    { return EPOLLIN; }
    virtual MyList* CB(MyEvent*, int*) override
    { return nullptr; }
    virtual void SetEpollEvents(uint32_t ev) override
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
    MyList            m_send;              // timer定时通知消息队列
    /* idx: 0 used by MyWorker, 1 used by MyApp */
    int               m_sockpair[2];
};

#endif
