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
class MyModule;
class MyModManager;
class MySocksMgr;
class MyHandleMgr;
class MyWorker;
class MyTimerTask;

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
    bool CreateContext(std::shared_ptr<MyModule>& mod_inst, const std::string& params);

    MyContext* GetContext(uint32_t handle);
    MyContext* GetContext(std::string& service_name);
    std::shared_ptr<MySocksMgr> GetSocksMgr();
    MyTimerTask* GetTimerTask() { return m_timer_task; }

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
    void Start(int worker_count);
    void StartWorker(int worker_count);
    void StartTimerTask();
    void CheckStopWorkers(bool onethread = false);
    MyContext* GetContextWithMsg(bool onethread = false);
    void DispatchMsg(MyList* msg_list);
    void DispatchMsg(MyContext* context);
    void ProcessEvent(struct epoll_event *evs, int ev_count);
    void ProcessWorkerEvent(MyWorker *worker);
    void ProcessTimerEvent(MyTimerTask *timer_task);
    bool LoadFromConf(std::string& filename);
    void HandleSysMsg(MyMsg* msg);

    MyList              m_idle_workers;       // 空闲线程链表
    MyList              m_iidle_workers;      // 独立线程空闲列表
    MyList              m_cache_que;          // 缓存消息队列
    int                 m_epoll_fd;           // epoll文件描述符
    int                 m_cur_worker_count;   // 工作线程数
    bool                m_quit;               // 退出标志
    std::shared_ptr<MyHandleMgr> _handle_mgr; // 句柄管理对象
    std::shared_ptr<MyModManager> _mods;      // 模块管理对象
    std::shared_ptr<MySocksMgr> _socks_mgr;   // 套接字管理对象
    std::string         m_mod_path;           // 模块路径
    MyTimerTask*        m_timer_task;         // 定时器线程对象
    std::mutex          m_mutex;
};

#endif
