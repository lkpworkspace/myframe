#include <errno.h>
#include <signal.h>
#include <sys/epoll.h>
#include "MyApp.h"
#include "MyWorker.h"
#include "MyLog.h"
#include "MyCUtils.h"
#include "MyContext.h"
#include "MyMsg.h"
#include "MyModule.h"

MyApp::MyApp(int worker_count) :
    m_epoll_fd(-1),
    m_worker_count(0),
    m_quit(true)
{
    SetInherits("MyObj");
    Start(worker_count);
}

MyApp::~MyApp()
{}

/**
 * 创建一个新的服务:
 *      1. 从MyModules中获得对应模块对象
 *      2. 生成MyContext
 *      3. 将模块对象加入MyContext对象
 *      4. 并MyContext加入Context数组
 *      3. 注册句柄
 *      4. 初始化服务
*/
bool MyApp::CreateContext(const char* mod_path, const char* mod_name, const char* param)
{
    m_mods.SetModPath(mod_path);
    if(false == m_mods.LoadMod(mod_name)) return false;
    MyModule* mod_inst = m_mods.CreateModInst(mod_name);
    return CreateContext(mod_inst, param);
}

bool MyApp::CreateContext(MyModule* mod_inst, const char* param)
{
    MyContext* ctx = new MyContext(mod_inst);
    m_handle_mgr.RegHandle(ctx);
    ctx->Init(param);
    // 初始化之后, 手动将服务中发送消息队列分发出去
    ctx->m_recv.Append(&ctx->m_send);
    return true;
}

bool MyApp::AddEvent(MyEvent* ev)
{
    int ret = true;
    struct epoll_event event;

    event.data.ptr = ev;
    event.events = ev->GetEpollEventType();
    // 如果该事件已经注册，就修改事件类型
    if(-1 == epoll_ctl(m_epoll_fd,EPOLL_CTL_MOD,ev->GetFd(),&event)) {
        // 没有注册就添加至epoll
        if(-1 == epoll_ctl(m_epoll_fd,EPOLL_CTL_ADD,ev->GetFd(),&event)){
            ret = false;
            MYLOG(MYLL_ERROR,("%s\n", my_get_error()));
        }
    }else{
        MYLOG(MYLL_WARN,("%p has already reg ev\n", ev));
    }
    return ret;
}

bool MyApp::DelEvent(MyEvent *ev)
{
    int ret = true;
    if(-1 == epoll_ctl(m_epoll_fd,EPOLL_CTL_DEL,ev->GetFd(),NULL)){
        ret = false;
        MYLOG(MYLL_ERROR, ("%s\n",my_get_error()));
    }
    return ret;
}

void MyApp::StartWorker(int worker_count)
{
    for(int i = 0; i < worker_count; ++i){
        MyWorker* worker = new MyWorker();
        worker->Start();
        AddEvent(static_cast<MyEvent*>(worker));
        m_worker_count++;
    }
    MYLOG(MYLL_INFO, ("Worker Start\n"));
}

void MyApp::Start(int worker_count)
{
    m_epoll_fd = epoll_create(1024);
    if(-1 == m_epoll_fd){
        MYLOG(MYLL_ERROR,("%s\n", my_get_error()));
        return;
    }
    MYLOG(MYLL_INFO, ("create epoll fd %d\n", m_epoll_fd));

    StartWorker(worker_count);

    // ingore SIGPIPE signal
    signal(SIGPIPE,SIG_IGN);
    MYLOG(MYLL_INFO, ("ingore SIGPIPE signal\n"));
    m_quit = false;
}

// 获得一个有消息待处理的服务(m_recv不为空的服务)
MyContext* MyApp::GetContext()
{
    // 1. 判断是否在全局数组中
    //      - 如果不是则查找下一个
    // 2. 如果在全局数组中，查看是否有消息
    //      -  有则返回，没有继续判断下一个
    // 3. 如果遍历一遍都没有就直接返回nullptr
    MyContext* ctx;
    MyContext* temp;
    ctx = temp = m_handle_mgr.GetNextContext();
    if(ctx == nullptr) return nullptr;
    for(;;){
        if(ctx->m_in_global){
            if(false == ctx->m_recv.IsEmpty()){
                return ctx;
            }
        }
        ctx = m_handle_mgr.GetNextContext();
        if(temp == ctx) break;
    }
    return nullptr;
}

// 将获得的消息分发给其他服务
void MyApp::DispatchMsg(MyContext* context)
{
    MyMsg* msg;
    MyContext* ctx;
    MyNode* begin;
    MyNode* end;
    MyNode* temp;
    if(nullptr == context) return;
    MyList* msg_list = context->GetDispatchMsgList();

    begin= msg_list->Begin();
    end = msg_list->End();
    for(;begin != end;)
    {
        temp = begin->next;
        msg = static_cast<MyMsg*>(begin);
        if(msg->destination == 0xffffff){
            MYLOG(MYLL_INFO, ("handle 0xffffff msg\n"));
            // TODO...
        }else{
            ctx = m_handle_mgr.GetContext(msg->destination);
            if(nullptr != ctx){
                msg_list->Del(begin);
                ctx->PushMsg(begin);
            }else{
                msg_list->Del(begin);
                MYLOG(MYLL_ERROR, ("err handle %u\n", msg->destination));
            }
        }
        begin = temp;
    }
}

void MyApp::CheckStopWorkers()
{
    char cmd = 'y';

    MyWorker* worker;
    MyContext* context;
    MyNode* begin;
    MyNode* end;
    MyNode* temp;
    MyList* list;

    begin= m_idle_workers.Begin();
    end = m_idle_workers.End();
    for(;begin != end;)
    {
        temp = begin->next;
        worker = static_cast<MyWorker*>(begin);
        if(nullptr == (context = GetContext())) break;
        list = context->GetRecvMsgList();
        if(!list->IsEmpty()){
            worker->m_que.Append(list);
            context->m_in_global = false;
            worker->SetContext(context);
            m_idle_workers.Del(begin);
            worker->SendCmd(&cmd, sizeof(char));
        }else{
            MYLOG(MYLL_ERROR, ("context has no msg\n"));
        }
        begin = temp;
    }
}

void MyApp::ProcessWorkerEvent(MyWorker* worker)
{
    char cmd = '\0';
    worker->RecvCmd(&cmd, 1);
    switch(cmd){
    case 'i': // idle
        // 1. 将服务要发送的消息队列分发至各个服务的接受队列
        // 2. 将工作线程中的服务状态设置为全局状态
        // 3. 将线程加入空闲队列

        // 1.
        DispatchMsg(worker->m_context);
        // 2.
        worker->Idle();
        // 3.
        m_idle_workers.AddTail(static_cast<MyNode*>(worker));
        break;
    case 'q': // quit
        // TODO...
        break;
    default:
        MYLOG(MYLL_ERROR, ("get worker cmd err %c\n", cmd));
        break;
    }
}

void MyApp::ProcessEvent(struct epoll_event* evs, int ev_count)
{
    MyNode* ev_obj;

    for(int i = 0; i < ev_count; ++i)
    {
        ev_obj = static_cast<MyNode*>(evs[i].data.ptr);
        switch(ev_obj->GetNodeType())
        {
        case MyNode::NODE_EVENT:
            ProcessWorkerEvent(static_cast<MyWorker*>(ev_obj));
            break;
        case MyNode::NODE_MSG:
            // 现在貌似不会产生该类型的消息
            // 将来可能socket消息使用该类型或者 NODE_SOCKET 类型
            // TODO...
            break;
        default:
            MYLOG(MYLL_ERROR,("node type error\n")); break;
        }
    }
}

int MyApp::Exec()
{
    int ev_count = 0;
    int max_ev_count = 64;
    int time_wait = 100;
    struct epoll_event* evs;

    evs = (struct epoll_event*)malloc(sizeof(struct epoll_event) * max_ev_count);
    while(m_worker_count)
    {
        // 1. 检查空闲工作线程
        //  - 检查队列是否有空闲线程
        //      - 如果有就找到一个有消息的服务:
        //          - 将发送消息队列分发至其他服务
        //              - 如果是申请系统操作消息(dst id: 0xffffff)
        //                  - 由主线程处理该消息(如: 创建服务， 删除服务， 。。。)
        //              - 如果是普通消息，就分发就行
        //          - 将接收消息链表移动至工作线程
        //          - 标记服务进入工作线程
        //          - 设置工作线程要处理的服务对象指针
        //          - 唤醒工作线程并工作
        //      - 如果没有,退出检查
        CheckStopWorkers();
        // 2. 等待事件
        if(0 <= (ev_count = epoll_wait(m_epoll_fd, evs, max_ev_count, time_wait))) {
            // 3. 处理事件
            //  - 获得事件区分事件类型
            //      - 线程事件
            //          - 退出消息
            //              - TODO...
            //          - 线程事件处理完毕
            //              - 将线程放入空闲线程链表
            //      - msg事件
            //          - 如果是socket消息
            //              - TODO...
            ProcessEvent(evs, ev_count);
        }else{
            MYLOG(MYLL_ERROR,("%s\n", my_get_error()));
        }
        // 4. loop
    }

    // quit MyApp
    free(evs);
    close(m_epoll_fd);

    MYLOG(MYLL_INFO, ("MyApp Exit\n"));
    return 0;
}


void MyApp::Quit()
{
    // TODO...
    return;
}
