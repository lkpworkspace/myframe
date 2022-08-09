#pragma once

#include "MyCommon.h"
#include "MyWorker.h"

class MyMsg;
class MyContext;
class MyWorkerCommon : public MyWorker
{
    friend class MyApp;
public:
    MyWorkerCommon();
    ~MyWorkerCommon();

    /// override MyWorker virtual method
    void Run() override;
    void OnInit() override;
    void OnExit() override;

    /// override MyEvent virtual method
    MyEventType GetMyEventType() { return MyEventType::EV_WORKER; }

    void SetContext(MyContext* context){ _context = context; }

private:
    /* 工作线程消息处理 */
    int Work();
    /* 工作线程进入空闲链表之前进行的操作 */
    void Idle();
    /// 运行时消息队列
    std::list<std::shared_ptr<MyMsg>> _que;
    //// 当前执行服务的指针
    MyContext* _context;
};
