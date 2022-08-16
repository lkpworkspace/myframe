#pragma once
#include <sys/epoll.h>

#include <pthread.h>
#include <unistd.h>

#include <atomic>
#include <list>
#include <memory>

#include "MyEvent.h"
#include "MyMsg.h"

enum class MyWorkerCmd : char {
    IDLE            = 'i',     ///< 工作线程空闲
    RUN             = 'r',     ///< 工作线程运行
    WAIT_FOR_MSG    = 'w',     ///< 工作线程等待消息
    QUIT            = 'q',     ///< 线程退出命令
};

class MyWorker : public MyEvent
{
    friend class MyApp;
public:
    MyWorker();
    virtual ~MyWorker();

    ////////////////////////////// thread 相关函数
    virtual void OnInit() {}
    virtual void Run() = 0;
    virtual void OnExit(){}
    void Start();
    void Stop();
    bool IsRuning() { return _runing; }
    pthread_t GetPosixThreadId(){return _posix_thread_id;}

    ////////////////////////////// event 相关函数
    int GetFd() override { return _sockpair[1]; }
    unsigned int ListenEpollEventType() override { return EPOLLIN; }
    void RetEpollEventType(uint32_t ev) override { ev = ev; }

    ////////////////////////////// 线程间通信相关函数
    int SendCmdToWorker(const MyWorkerCmd& cmd);
    int RecvCmdFromWorker(MyWorkerCmd& cmd);

    ////////////////////////////// 消息处理相关函数
    int SendMsgListSize() { return _send.size(); }
    void SendMsg(const std::string& dst, std::shared_ptr<MyMsg> msg);

    void SetInstName(const std::string& name) { _inst_name = name; }
    std::string& GetInstName() { return _inst_name; }

protected:
    int DispatchMsg();
    int DispatchAndWaitMsg();
    
    int RecvCmdFromMain(MyWorkerCmd& cmd);
    int SendCmdToMain(const MyWorkerCmd& cmd);
    
    int RecvMsgListSize() { return _que.size(); }
    const std::shared_ptr<const MyMsg> GetRecvMsg();
    
    void PushSendMsgList(std::list<std::shared_ptr<MyMsg>>& msg_list);
    
    static void* ListenThread(void*);

private:
    bool CreateSockPair();
    void CloseSockPair();

    std::string _inst_name;
    /// idx: 0 used by MyWorkerCommon, 1 used by MyApp
    int _sockpair[2];
    /// 接收消息队列
    std::list<std::shared_ptr<MyMsg>> _recv;
    /// 运行时消息队列
    std::list<std::shared_ptr<MyMsg>> _que;
    /// 发送消息队列
    std::list<std::shared_ptr<MyMsg>> _send;
    /// posix thread id
    pthread_t _posix_thread_id;
    std::atomic_bool _runing;

};

extern "C" {
    typedef MyWorker* (*my_worker_create_func)(const std::string&);
} // extern "C"