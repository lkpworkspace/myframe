#ifndef __MyThread_H__
#define __MyThread_H__
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "MyCommon.h"
#include "MyEvent.h"

class MyThread : public MyEvent
{
public:

    MyThread();

    virtual ~MyThread();

    virtual void Run() = 0;

    // 子类需要先调用父类的OnInit()虚函数
    virtual void OnInit(){m_thread_id = syscall(SYS_gettid);}

    // 子类需要最后调用父类的OnExit()虚函数
    virtual void OnExit(){}

    void Start();

    void Stop();

    bool IsRuning() { return m_runing; }

    // 获得linux线程的id
    pid_t GetThreadId() {return m_thread_id;}

    // 获得posix的线程id
    pthread_t GetPosixThreadId(){return m_posix_thread_id;}

    virtual int GetEventType() { return EV_THREAD; }

protected:

    static void* ListenThread(void*);

    /* posix thread id */
    pthread_t               m_posix_thread_id;

    /* system thread id */
    pid_t                   m_thread_id;

    volatile bool           m_runing;
};
#endif
