#include <gtest/gtest.h>
#include "MyThread.h"

class MyThreadTest : public MyThread
{
public:
    MyThreadTest():
        m_val(0)
    {}
    ~MyThreadTest(){}

    /* 事件类型 */
    virtual int GetEventType() override
    { return EV_THREAD; }

    virtual unsigned int GetEpollEventType() override
    { return EPOLLIN; }

    virtual void SetEpollEvents(uint32_t) override
    {}

    virtual MyList* CB(MyEvent*, int*) override
    { return nullptr; }

    virtual void Run()
    {
        m_val = 100;
    }

    int m_val;
};

TEST(MyThread, Inherits)
{
    MyThreadTest thread;
    EXPECT_EQ(true, thread.Inherits("MyEvent"));
    EXPECT_EQ(true, thread.Inherits("MyNode"));
    EXPECT_EQ(true, thread.Inherits("MyObj"));
}

/**
 * 1. 启动线程执行任务
 * 2. 停止线程判断值是否改变
 */

TEST(MyThread, Run)
{
    MyThreadTest thread;
    thread.Start();
    usleep(1000 * 100);
    thread.Stop();
    EXPECT_EQ(100, thread.m_val);
}
