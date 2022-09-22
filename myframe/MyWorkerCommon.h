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
    MyEventType GetMyEventType() { return MyEventType::WORKER_COMMON; }

    void SetContext(std::shared_ptr<MyContext> context) { _context = context; }
    std::shared_ptr<MyContext> GetContext() {return (_context.expired() ? nullptr : _context.lock()); }
    
private:
    /* 工作线程消息处理 */
    int Work();
    /* 工作线程进入空闲链表之前进行的操作 */
    void Idle();
    //// 当前执行actor的指针
    std::weak_ptr<MyContext> _context;
};
