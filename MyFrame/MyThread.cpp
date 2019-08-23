#include "MyThread.h"

#include <boost/log/trivial.hpp>

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
            BOOST_LOG_TRIVIAL(error) << "pthread create failed";
            return;
        }
        res = pthread_detach(m_posix_thread_id);
        if(res != 0)
        {
            m_runing = false;
            BOOST_LOG_TRIVIAL(error) << "pthread detach failed";
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
