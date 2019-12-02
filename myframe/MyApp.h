#ifndef __MYAPP_H__
#define __MYAPP_H__
#include <vector>
#include "MyCommon.h"
#include "MyList.h"

struct epoll_event;
class MyMsg;
class MyEvent;
class MyContext;
class MyModule;
class MyModules;
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

    bool ParseArg(int argc, char** argv);

    bool CreateContext(const char* mod_path, const char* mod_name, const char* service_name, const char* param);
    bool CreateContext(const char* mod_name, const char* service_name, const char* param);
    bool CreateContext(MyModule* mod_inst, const char* param);

    MyContext* GetContext(uint32_t handle);
    MyContext* GetContext(std::string& service_name);
    MySocksMgr* GetSocksMgr();
    MyTimerTask* GetTimerTask() { return m_timer_task; }

    bool AddEvent(MyEvent *ev);
    bool DelEvent(MyEvent *ev);

    int Exec();                           // mainloop
    void Quit();                          // exit this app

private:
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

    MyList              m_idle_workers;   // 空闲线程链表
    MyList              m_iidle_workers;  // 独立线程空闲列表
    MyList              m_cache_que;      // 缓存消息队列
    int                 m_epoll_fd;       // epoll文件描述符
    int                 m_worker_count;   // 工作线程数
    int                 m_worker_count_conf;
    bool                m_quit;           // 退出标志
    MyHandleMgr*        m_handle_mgr;     // 句柄管理对象
    MyModules*          m_mods;           // 模块管理对象
    std::string         m_mod_path;       // 模块路径
    MySocksMgr*         m_socks_mgr;      // 套接字管理对象
    MyTimerTask*        m_timer_task;     // 定时器线程对象
};

#endif
