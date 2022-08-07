#ifndef __MYWORKER_H__
#define __MYWORKER_H__

#include "MyCommon.h"
#include "MyThread.h"

class MyMsg;
class MyContext;
class MyWorker : public MyThread
{
    friend class MyApp;
public:
    enum class MyWorkerCmdType : char {
        NONE            = '\0',    // 未知命令
        IDLE            = 'i',     // 工作线程空闲
        QUIT            = 'q',     // 线程退出命令
    };

    MyWorker();
    ~MyWorker();

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
    { return EV_WORKER; }
    int GetFd() override;
    unsigned int ListenEpollEventType() override;
    void RetEpollEventType(uint32_t ev) override
    { ev = ev; }

    // 主线程调用该函数与工作线程通信
    int SendCmd(const char* cmd, size_t len);
    int RecvCmd(char* cmd, size_t len);

    void SetCmd(MyWorkerCmdType cmd){ _cmd = cmd; }
    void SetContext(MyContext* context){ _context = context; }

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

    /// idx: 0 used by MyWorker, 1 used by MyApp
    int _sockpair[2];
    /// 发送消息队列(针对没有服务的消息,缓存到该队列)
    std::list<std::shared_ptr<MyMsg>> _send;
    /// 运行时消息队列
    std::list<std::shared_ptr<MyMsg>> _que;
    //// 当前执行服务的指针
    MyContext* _context;
    /// 当前命令类型
    MyWorkerCmdType _cmd;
};

#endif
