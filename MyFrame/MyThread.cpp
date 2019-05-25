#include "MyThread.h"
#include "MyLog.h"

MyThread::MyThread() :
    m_posix_thread_id(-1),
    m_thread_id(-1),
    m_runing(false)
{
    SetInherits("MyEvent");
}

MyThread::~MyThread()
{
    Stop();
}

void MyThread::Start()
{
    int res = 0;
    if(m_runing == false)
    {
        m_runing = true;
        res = pthread_create(&m_posix_thread_id, NULL, &MyThread::ListenThread, this);
        if(res != 0)
        {
            m_runing = false;
            MYLOG(MYLL_ERROR, ("pthread create fail\n"));
            return;
        }
        res = pthread_detach(m_posix_thread_id);
        if(res != 0)
        {
            m_runing = false;
            MYLOG(MYLL_ERROR, ("pthread detach fail\n"));
            return;
        }
    }
}

void MyThread::Stop()
{
    m_runing = false;
}

void* MyThread::ListenThread(void* obj)
{
    MyThread* t = static_cast<MyThread*>(obj);
    t->OnInit();
    while (t->m_runing)
		t->Run();
	t->OnExit();
    return NULL;
}
