#pragma once
#include <pthread.h>

#include <atomic>
#include <memory>
#include <list>
#include <unordered_map>

class MyMsg;
class MyWorker;
class MyWorkerManager final {
public:
    MyWorkerManager();
    virtual ~MyWorkerManager();

    int WorkerSize();

    std::shared_ptr<MyWorker> Get(int fd);
    std::shared_ptr<MyWorker> Get(const std::string&);
    bool Add(std::shared_ptr<MyWorker> worker);
    void Del(std::shared_ptr<MyWorker> worker);

    int IdleWorkerSize();
    std::shared_ptr<MyWorker> FrontIdleWorker();
    void PopFrontIdleWorker();
    void PushBackIdleWorker(std::shared_ptr<MyWorker> worker);

    void PushWaitWorker(std::shared_ptr<MyWorker>& worker);
    void WeakupWorker();

    void DispatchWorkerMsg(std::shared_ptr<MyMsg>& msg);

private:
    /// 工作线程数(包含用户线程)
    std::atomic_int _cur_worker_count = {0};
    /// 读写锁
    pthread_rwlock_t _rw;
    /// 空闲线程链表
    std::list<std::weak_ptr<MyWorker>> _idle_workers;
    /// 有消息线程
    std::list<std::weak_ptr<MyWorker>> _weakup_workers;
    /// name/handle 映射表
    std::unordered_map<std::string, int> _name_handle_map;
    /// handle/worker 映射表
    std::unordered_map<int, std::shared_ptr<MyWorker>> _workers;

};
