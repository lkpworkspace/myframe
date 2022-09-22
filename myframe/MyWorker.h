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
    IDLE            = 'i',     ///< worker空闲(worker发送的指令)
    WAIT_FOR_MSG    = 'w',     ///< worker等待消息(worker发送的指令)
    RUN             = 'r',     ///< worker运行(main回复的指令)
    QUIT            = 'q',     ///< worker退出(main回复的指令)
};

enum class MyWorkerCtrlOwner : int {
    MAIN,
    WORKER,
};

class MyWorker : public MyEvent
{
    friend class MyApp;
    friend class MyModLib;
    friend class MyModManager;
    friend class MyWorkerManager;
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

    ////////////////////////////// 接收worker/actor消息
    int RecvMsgListSize() { return _que.size(); }
    const std::shared_ptr<const MyMsg> GetRecvMsg();

    ////////////////////////////// 发送消息相关函数
    int SendMsgListSize() { return _send.size(); }
    void SendMsg(const std::string& dst, std::shared_ptr<MyMsg> msg);
    void PushSendMsgList(std::list<std::shared_ptr<MyMsg>>& msg_list);

    ////////////////////////////// 接收/发送主线程控制消息
    int RecvCmdFromMain(MyWorkerCmd& cmd, int timeout_ms = -1);
    int SendCmdToMain(const MyWorkerCmd& cmd);
    /// 分发消息并立即返回
    int DispatchMsg();
    /// 分发消息并等待回复消息
    int DispatchAndWaitMsg();
    
    /// worker fd
    int GetWorkerFd() { return _sockpair[0]; }

    const std::string& GetModName() const { return _mod_name; }
    const std::string& GetTypeName() const { return _worker_name; }
    const std::string& GetInstName() const { return _inst_name; }
    const std::string GetWorkerName() const;

private:
    static void* ListenThread(void*);
    
    ////////////////////////////// 线程间通信相关函数
    int SendCmdToWorker(const MyWorkerCmd& cmd);
    int RecvCmdFromWorker(MyWorkerCmd& cmd);

    ////////////////////////////// 线程交互控制flag函数
    void SetCtrlOwnerFlag(MyWorkerCtrlOwner owner) { _ctrl_owner = owner; }
    MyWorkerCtrlOwner GetOwner() const { return _ctrl_owner; }
    void SetWaitMsgQueueFlag(bool in_wait_msg_queue) { _in_msg_wait_queue = in_wait_msg_queue; }
    bool IsInWaitMsgQueue() { return _in_msg_wait_queue; }

    ////////////////////////////// worker name
    void SetModName(const std::string& name) { _mod_name = name; }
    void SetTypeName(const std::string& name) { _worker_name = name; }
    void SetInstName(const std::string& name) { _inst_name = name; }

    bool CreateSockPair();
    void CloseSockPair();

    /// worker name
    std::string _mod_name;
    std::string _worker_name;
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
    /// state
    std::atomic_bool _runing;
    MyWorkerCtrlOwner _ctrl_owner{MyWorkerCtrlOwner::MAIN};
    bool _in_msg_wait_queue{false};
};

extern "C" {
    typedef std::shared_ptr<MyWorker> (*my_worker_create_func)(const std::string&);
} // extern "C"