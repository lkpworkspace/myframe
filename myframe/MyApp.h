#ifndef __MYAPP_H__
#define __MYAPP_H__
#include <memory>
#include <vector>
#include <mutex>

#include "MyCommon.h"
#include "MyList.h"

struct epoll_event;
class MyMsg;
class MyEvent;
class MyContext;
class MyActor;
class MyWorker;
class MyModManager;
class MyHandleManager;
class MyWorkerCommon;
class MyWorkerTimer;

/**
 * 该类为单例类，不允许创建多个
 */
class MyApp : public MyObj
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
        const std::string& service_name, 
        const std::string& instance_name, 
        const std::string& params);
    bool CreateContext(std::shared_ptr<MyActor>& mod_inst, const std::string& params);

    MyContext* GetContext(uint32_t handle);
    MyContext* GetContext(std::string& service_name);

    MyWorkerTimer* GetTimerWorker() { return _timer_worker; }

    bool AddEvent(MyEvent *ev);
    bool DelEvent(MyEvent *ev);

    int Exec();                           // mainloop

public: // for ut
    bool LoadModsFromConf(const std::string& path);

private:
    bool LoadServiceFromLib(
        const Json::Value& root, 
        const Json::Value& service_list, 
        const std::string& service_name);
    bool LoadServiceFromClass(
        const Json::Value& root, 
        const Json::Value& service_list, 
        const std::string& service_name);
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

    /// 获取有消息的服务
    MyContext* GetContextWithMsg();
    /// 通知执行事件
    void CheckStopWorkers();
    /// 分发事件
    void DispatchMsg(std::list<std::shared_ptr<MyMsg>>& msg_list);
    void DispatchMsg(MyContext* context);
    void ProcessEvent(struct epoll_event *evs, int ev_count);
    void ProcessWorkerEvent(MyWorkerCommon*);
    void ProcessTimerEvent(MyWorkerTimer*);
    void ProcessUserEvent(MyWorker*);
    void HandleSysMsg(std::shared_ptr<MyMsg>& msg);

    /// 退出标志
    bool _quit;
    /// epoll文件描述符
    int _epoll_fd;
    /// 工作线程数
    int _cur_worker_count;
    /// 空闲线程链表
    MyList _idle_workers;
    /// 缓存消息队列
    std::list<std::shared_ptr<MyMsg>> _cache_que;          
    /// 句柄管理对象
    std::shared_ptr<MyHandleManager> _handle_mgr; 
    /// 模块管理对象
    std::shared_ptr<MyModManager> _mods;
    /// 定时器线程对象      
    MyWorkerTimer* _timer_worker;

};

#endif
