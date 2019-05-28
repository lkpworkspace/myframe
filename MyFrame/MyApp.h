#ifndef __MYAPP_H__
#define __MYAPP_H__

#include "MyCommon.h"
#include "MyList.h"
#include "MyHandleMgr.h"
#include "MyModules.h"

struct epoll_event;
class MyEvent;
class MyContext;
class MyWorker;
/**
 * 该类为单例类，不允许创建多个
 */
class MyApp : public MyObj
{
public:
    MyApp(int worker_count = 1);
    virtual ~MyApp();

    static MyApp* s_inst;

    bool CreateContext(const char* mod_path, const char* mod_name, const char* param);
    bool CreateContext(MyModule* mod_inst, const char* param);

    MyContext* GetContext(uint32_t handle);

    bool AddEvent(MyEvent *ev);
    bool DelEvent(MyEvent *ev);

    int Exec();                           // mainloop
    void Quit();                          // exit this app

private:
    void Start(int worker_count);
    void StartWorker(int worker_count);
    void CheckStopWorkers();
    MyContext* GetContextWithMsg();
    void DispatchMsg(MyContext* context);
    void DispatchMsg(MyList* msg_list);
    void ProcessEvent(struct epoll_event *evs, int ev_count);
    void ProcessWorkerEvent(MyWorker *worker);

    MyList              m_idle_workers;   // 空闲线程链表
    MyList              m_cache_que;      // 缓存消息队列
    int                 m_epoll_fd;       // epoll文件描述符
    int                 m_worker_count;   // 工作线程数
    bool                m_quit;           // 退出标志
    MyHandleMgr         m_handle_mgr;     // 句柄管理对象
    MyModules           m_mods;           // 模块管理对象
};

#endif
