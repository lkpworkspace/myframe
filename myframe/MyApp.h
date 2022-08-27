#ifndef __MYAPP_H__
#define __MYAPP_H__
#include <memory>
#include <atomic>
#include <vector>
#include <mutex>
#include <list>
#include <unordered_map>

#include <jsoncpp/json/json.h>

struct epoll_event;
class MyContext;
class MyMsg;
class MyActor;
class MyEvent;
class MyWorker;
class MyWorkerCommon;
class MyWorkerTimer;
class MyModManager;
class MyContextManager;
class MyWorkerManager;

/**
 * 该类为单例类，不允许创建多个
 */
class MyApp
{
private:
    MyApp();
    virtual ~MyApp();
    static MyApp* s_inst;
public:
    static MyApp* Create();
    static MyApp* Inst();

    bool Init();

    bool CreateContext(
        const std::string& mod_name, 
        const std::string& actor_name, 
        const std::string& instance_name, 
        const std::string& params);
    bool CreateContext(std::shared_ptr<MyActor>& mod_inst, const std::string& params);

    MyContext* GetContext(uint32_t handle);
    MyContext* GetContext(std::string& actor_name);

    bool AddWorker(std::shared_ptr<MyWorker> worker);

    std::shared_ptr<MyWorkerTimer> GetTimerWorker();
    std::shared_ptr<MyContextManager>& GetHandleManager() { return _context_mgr; }

    bool AddEvent(std::shared_ptr<MyEvent> ev);
    bool DelEvent(std::shared_ptr<MyEvent> ev);

    int Exec();                           // mainloop

public: // for ut
    bool LoadModsFromConf(const std::string& path);

private:
    bool LoadActorFromLib(
        const Json::Value& root, 
        const Json::Value& actor_list, 
        const std::string& actor_name);
    bool LoadActorFromClass(
        const Json::Value& root, 
        const Json::Value& actor_list, 
        const std::string& actor_name);
    bool LoadWorkerFromLib(
        const Json::Value& root, 
        const Json::Value& worker_list, 
        const std::string& worker_name);
    bool LoadWorkerFromClass(
        const Json::Value& root, 
        const Json::Value& worker_list, 
        const std::string& worker_name);
    /// worker
    void Start(int worker_count);
    void StartCommonWorker(int worker_count);
    void StartTimerWorker();

    /// 通知执行事件
    void CheckStopWorkers();
    /// 分发事件
    void DispatchMsg(std::list<std::shared_ptr<MyMsg>>& msg_list);
    void DispatchMsg(MyContext* context);
    void ProcessEvent(struct epoll_event *evs, int ev_count);
    void ProcessWorkerEvent(std::shared_ptr<MyWorkerCommon>);
    void ProcessTimerEvent(std::shared_ptr<MyWorkerTimer>);
    void ProcessUserEvent(std::shared_ptr<MyWorker>);

    /// 退出标志
    std::atomic_bool _quit = {true};
    /// epoll文件描述符
    int _epoll_fd;
    /// 句柄管理对象
    std::shared_ptr<MyContextManager> _context_mgr; 
    /// 模块管理对象
    std::shared_ptr<MyModManager> _mods;
    /// 线程管理对象
    std::shared_ptr<MyWorkerManager> _worker_mgr;

};

#endif
