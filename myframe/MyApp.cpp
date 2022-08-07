#include "MyApp.h"

#include <errno.h>
#include <signal.h>
#include <sys/epoll.h>

#include <iostream>

#include <jsoncpp/json/json.h>

#include "MyFlags.h"
#include "MyLog.h"
#include "MyWorker.h"
#include "MyCUtils.h"
#include "MyContext.h"
#include "MyMsg.h"
#include "MyModule.h"
#include "MyHandleMgr.h"
#include "MySocksMgr.h"
#include "MyModManager.h"
#include "MyTimerTask.h"

MyApp* MyApp::s_inst = nullptr;

MyApp* MyApp::Create() {
    if(s_inst == nullptr){
        s_inst = new MyApp();
    }
    return s_inst;
}

MyApp* MyApp::Inst() {
    return s_inst;
}

MyApp::MyApp() :
    m_epoll_fd(-1),
    m_cur_worker_count(0),
    m_quit(true) {
    SetInherits("MyObj");
    s_inst = this;
}

MyApp::~MyApp()
{}

bool MyApp::Init() {
    _handle_mgr = std::make_shared<MyHandleMgr>();
    _mods = std::make_shared<MyModManager>();
    _socks_mgr = std::make_shared<MySocksMgr>();

    /// load modules
    if (!LoadModsFromConf(FLAGS_service_desc_path)) {
        return false;
    }

    /// start worker
    Start(FLAGS_worker_count);
    return true;
}

bool MyApp::LoadModsFromConf(const std::string& path) {
    auto service_conf_list = MyCommon::GetDirFiles(path);
    LOG(INFO) << "Search " << service_conf_list.size() << " service conf";
    if (service_conf_list.size() <= 0) {
        LOG(WARNING) << "Search service failed, exit";
        return false;
    }
    bool res = false;
    for (const auto& it : service_conf_list) {
        LOG(INFO) << "Load " << it << " ...";
        auto root = MyCommon::LoadJsonFromFile(it);
        if (root.isNull()) {
            LOG(ERROR) << it <<" parse failed, skip";
            continue;
        }
        if (!root.isMember("type") || !root["type"].isString()) {
            LOG(ERROR) << it <<" key \"type\": no key or not string, skip";
            continue;
        }
        const auto& type = root["type"].asString();
        // load service
        if (root.isMember("service") && root["service"].isObject()) {
            const auto& service_list = root["service"];
            Json::Value::Members service_name_list = service_list.getMemberNames();
            for (auto inst_name_it = service_name_list.begin(); inst_name_it != service_name_list.end(); ++inst_name_it) {
                LOG(INFO) << "Load service " << *inst_name_it << " ...";
                if (type == "library") {
                    res |= LoadServiceFromLib(root, service_list, *inst_name_it);
                } else if (type == "class") {
                    res |= LoadServiceFromClass(root, service_list, *inst_name_it);
                } else {
                    LOG(ERROR) << "Unknown type " << type;
                }
            }
        }
        // TODO worker
    }
    return res;
}

bool MyApp::LoadServiceFromLib(
    const Json::Value& root, 
    const Json::Value& service_list, 
    const std::string& service_name) {
    if (!root.isMember("lib") || !root["lib"].isString()) {
        LOG(ERROR) << "service " << service_name <<" key \"lib\": no key or not string, skip";
        return false;
    }
    const auto& lib_name = root["lib"].asString();
    if (!_mods->LoadMod(FLAGS_service_lib_path + lib_name)) {
        LOG(ERROR) << "load lib " << lib_name <<" failed, skip";
        return false;
    }
    
    const auto& insts = service_list[service_name];
    bool res = false;
    for (const auto& inst : insts) {
        LOG(INFO) << "create instance " << service_name << ": " << inst.toStyledString();
        if (!inst.isMember("instance_name")) {
            LOG(ERROR) << "service " << service_name <<" key \"instance_name\": no key, skip";
            return false;
        }
        if (!inst.isMember("instance_params")) {
            LOG(ERROR) << "service " << service_name <<" key \"instance_params\": no key, skip";
            return false;
        }
        res |= CreateContext(
            lib_name, service_name, 
            inst["instance_name"].asString(), 
            inst["instance_params"].asString());
    }
    return res;
}

bool MyApp::LoadServiceFromClass(
    const Json::Value& root, 
    const Json::Value& service_list, 
    const std::string& service_name) {
    const auto& insts = service_list[service_name];
    bool res = false;
    for (const auto& inst : insts) {
        LOG(INFO) << "create instance class" << ": " << inst.toStyledString();
        if (!inst.isMember("instance_name")) {
            LOG(ERROR) << "service " << service_name <<" key \"instance_name\": no key, skip";
            return false;
        }
        if (!inst.isMember("instance_params")) {
            LOG(ERROR) << "service " << service_name <<" key \"instance_params\": no key, skip";
            return false;
        }
        res |= CreateContext(
            "class", service_name, 
            inst["instance_name"].asString(), 
            inst["instance_params"].asString());
    }
    return res;
}

/**
 * 创建一个新的服务:
 *      1. 从MyModManager中获得对应模块对象
 *      2. 生成MyContext
 *      3. 将模块对象加入MyContext对象
 *      4. 将MyContext加入Context数组
 *      3. 注册句柄
 *      4. 初始化服务
*/
bool MyApp::CreateContext(
    const std::string& mod_name, 
    const std::string& service_name, 
    const std::string& instance_name, 
    const std::string& params) {
    auto mod_inst = _mods->CreateModInst(mod_name, service_name);
    if(mod_inst == nullptr) {
        LOG(ERROR) << "Create mod " << mod_name << "." << service_name << " failed";
        return false;
    }
    mod_inst->m_instance_name = instance_name;
    return CreateContext(mod_inst, params);
}

bool MyApp::CreateContext(std::shared_ptr<MyModule>& mod_inst, const std::string& params)
{
    MyContext* ctx = new MyContext(mod_inst);
    _handle_mgr->RegHandle(ctx);
    ctx->Init(params.c_str());
    // 初始化之后, 手动将服务中发送消息队列分发出去
    DispatchMsg(ctx);
    return true;
}

bool MyApp::AddEvent(MyEvent* ev)
{
    int ret = true;
    int res;
    struct epoll_event event;

    event.data.ptr = ev;
    event.events = ev->GetEpollEventType();

    LOG(INFO) << ev->GetObjName() << " reg event fd: " << ev->GetFd();
    // 如果该事件已经注册，就修改事件类型
    if(-1 == (res = epoll_ctl(m_epoll_fd,EPOLL_CTL_MOD,ev->GetFd(),&event))) {
        // 没有注册就添加至epoll
        if(-1 == (res = epoll_ctl(m_epoll_fd,EPOLL_CTL_ADD,ev->GetFd(),&event))){
            ret = false;
            LOG(ERROR) << "epoll " << my_get_error();
        }
    }else{
        LOG(WARNING) << ev->GetObjName() << " has already reg ev " 
            << ev->GetFd() << ": " << my_get_error();
    }
    return ret;
}

bool MyApp::DelEvent(MyEvent *ev)
{
    int ret = true;
    if(-1 == epoll_ctl(m_epoll_fd,EPOLL_CTL_DEL,ev->GetFd(),NULL)){
        ret = false;
        LOG(ERROR) << ev->GetObjName() << " del event " 
            << ev->GetFd() << ": " << my_get_error();
    }
    return ret;
}

void MyApp::StartWorker(int worker_count)
{
    for(int i = 0; i < worker_count; ++i){
        MyWorker* worker = new MyWorker();
        worker->Start();
        AddEvent(static_cast<MyEvent*>(worker));
        m_cur_worker_count++;
    }
    // 启动一个独立线程
    MyWorker* worker = new MyWorker();
    worker->SetCmd(MyWorker::MyWorkerCmdType::IDLE_ONE_THREAD);
    worker->Start();
    AddEvent(static_cast<MyEvent*>(worker));
    LOG(INFO) << "Common worker start";
}

void MyApp::StartTimerTask()
{
    m_timer_task = new MyTimerTask();
    m_timer_task->Start();
    AddEvent(static_cast<MyEvent*>(m_timer_task));
    LOG(INFO) << "Timer worker start";
}

void MyApp::Start(int worker_count)
{
    m_epoll_fd = epoll_create(1024);
    if(-1 == m_epoll_fd){
        LOG(ERROR) << my_get_error();
        return;
    }
    LOG(INFO) << "Create epoll fd " << m_epoll_fd;

    StartWorker(worker_count);
    StartTimerTask();

    // ingore SIGPIPE signal
    signal(SIGPIPE,SIG_IGN);
    LOG(INFO) << "Ingore SIGPIPE signal";
    m_quit = false;
}

MyContext* MyApp::GetContext(uint32_t handle)
{
    return _handle_mgr->GetContext(handle);
}

MyContext* MyApp::GetContext(std::string& service_name)
{
    return _handle_mgr->GetContext(service_name);
}

std::shared_ptr<MySocksMgr> MyApp::GetSocksMgr()
{ return _socks_mgr; }

// 获得一个有消息待处理的服务(m_recv不为空的服务)
MyContext* MyApp::GetContextWithMsg(bool onethread)
{
    return _handle_mgr->GetContext(onethread);
}

void MyApp::HandleSysMsg(MyMsg* msg)
{
    MyMsg::MyMsgType type = msg->GetMsgType();
    MySockMsg* smsg = nullptr;

    switch(type){
    case MyMsg::MyMsgType::SOCKET:
        // 接收一些socket的消息
        smsg = static_cast<MySockMsg*>(msg);
        if(smsg->GetSockMsgType() == MySockMsg::MySockMsgType::CLOSE){
            GetSocksMgr()->Close(smsg->GetSockId());
            LOG(INFO) << "main thread " 
                        << " close socket id: " << smsg->GetSockId();
        }
        break;
    default:
        LOG(INFO) << "main thread "
                        << " get unknown msg type: " << (int)msg->GetMsgType();
        break;
    }
}

void MyApp::DispatchMsg(MyList* msg_list)
{
    MyMsg* msg;
    MyContext* ctx;
    MyNode* begin;
    MyNode* end;
    MyNode* temp;

    begin= msg_list->Begin();
    end = msg_list->End();
    m_mutex.lock();
    for(;begin != end;)
    {
        temp = begin->next;
        msg = static_cast<MyMsg*>(begin);
        if(msg->destination == MY_FRAME_DST){
            msg_list->Del(begin);
            // 在此处理系统消息，不再转发给工作线程处理
            LOG(INFO) << "Handle MY_FRAME_DST msg";
            HandleSysMsg(msg);
        }else{
            ctx = _handle_mgr->GetContext(msg->destination);
            if(nullptr != ctx){
                msg_list->Del(begin);
                ctx->PushMsg(begin);
                _handle_mgr->PushContext(ctx);
            }else{
                msg_list->Del(begin);
                LOG(ERROR) << "Err msg src:" 
                    << msg->source << " dst:" << msg->destination 
                    << " session:" << msg->session;
                delete msg;
            }
        }
        begin = temp;
    }
    m_mutex.unlock();
}

// 将获得的消息分发给其他服务
void MyApp::DispatchMsg(MyContext* context)
{
    if(nullptr == context) return;
    context->m_in_global = true;
    MyList* msg_list = context->GetDispatchMsgList();
    DispatchMsg(msg_list);
}

void MyApp::CheckStopWorkers(bool onethread)
{
    char cmd = 'y';

    MyWorker* worker;
    MyContext* context;
    MyNode* begin;
    MyNode* end;
    MyNode* temp;
    MyList* list;

    MyList& idle_workers = onethread ? m_iidle_workers : m_idle_workers;
    begin= idle_workers.Begin();
    end = idle_workers.End();
    m_mutex.lock();
    for(;begin != end;)
    {
        temp = begin->next;
        worker = static_cast<MyWorker*>(begin);
        // 主线程有消息，则先派发执行主线程中消息
        if(!onethread && (false == m_cache_que.IsEmpty())){
            worker->m_que.Append(&m_cache_que);
            idle_workers.Del(worker);
            worker->SendCmd(&cmd, sizeof(char));
            begin = temp;
            continue;
        }
        // 主线程没有消息，则派发执行服务中的消息
        if(nullptr == (context = GetContextWithMsg(onethread))) break;
        list = context->GetRecvMsgList();
        if(!list->IsEmpty()){
            worker->m_que.Append(list);
            worker->SetContext(context);
            idle_workers.Del(begin);
            worker->SendCmd(&cmd, sizeof(char));
        }else{
            LOG(ERROR) << "Context has no msg";
        }
        begin = temp;
    }
    m_mutex.unlock();
}

void MyApp::ProcessTimerEvent(MyTimerTask *timer_task)
{
    char cmd = 'y';
    timer_task->RecvCmd(&cmd, 1);
    switch(cmd){
    case 'i': // idle
        // 将定时器线程的发送队列分发完毕
        DispatchMsg(&timer_task->m_send);
        timer_task->SendCmd(&cmd, sizeof(char));
        break;
    case 'q': // quit
        // TODO...
        LOG(WARNING) << "Unimplement timer task quit: " << cmd;
        break;
    default:
        LOG(WARNING) << "Unknown timer task cmd: " << cmd;
        break;
    }
}

void MyApp::ProcessWorkerEvent(MyWorker* worker)
{
    char cmd = 'y';
    worker->RecvCmd(&cmd, 1);
    switch(cmd){
    case 'i': // idle
        // 将工作线程的发送队列分发完毕
        DispatchMsg(&worker->m_send);

        // 将服务的发送队列分发完毕
        DispatchMsg(worker->m_context);

        if(false == m_cache_que.IsEmpty()){
            // 检查主线程消息缓存队列，如果有消息就分发给工作线程, 唤醒并执行
            worker->Idle();
            worker->m_que.Append(&m_cache_que);
            worker->SendCmd(&cmd, sizeof(char));
        }else{
            // 将工作线程中的服务状态设置为全局状态
            // 将线程加入空闲队列
            worker->Idle();
            m_idle_workers.AddTail(static_cast<MyNode*>(worker));
        }
        break;
    case 's': // 独立线程空闲
        // 将服务的发送队列分发完毕
        DispatchMsg(worker->m_context);
        worker->Idle();
        m_iidle_workers.AddTail(static_cast<MyNode*>(worker));
        break;
    case 'q': // quit
        // TODO...
        LOG(WARNING) << "Unimplement worker quit: " << cmd;
        break;
    default:
        LOG(WARNING) << "Unknown worker cmd: " << cmd;
        break;
    }
}

void MyApp::ProcessEvent(struct epoll_event* evs, int ev_count)
{
    MyEvent* ev_obj;

    for(int i = 0; i < ev_count; ++i)
    {
        ev_obj = static_cast<MyEvent*>(evs[i].data.ptr);
        ev_obj->SetEpollEvents(evs[i].events);
        if(MyNode::NODE_EVENT == ev_obj->GetNodeType()){
            switch(ev_obj->GetEventType()){
            case MyEvent::EV_WORKER:
                // 工作线程事件
                ProcessWorkerEvent(static_cast<MyWorker*>(ev_obj));break;
            case MyEvent::EV_TIMER:
                // 分发超时消息
                ProcessTimerEvent(static_cast<MyTimerTask*>(ev_obj));
                break;
            case MyEvent::EV_SOCK:
                // socket可读可写事件
                //      如果不是 EPOLLONESHOT 类型， 需要调用 DelEvent() 删除该监听事件
                //      将事件缓存到主线程的消息队列
                DelEvent(ev_obj);
                m_cache_que.AddTail(ev_obj);
                break;
            default:
                LOG(WARNING) << "Unknown event";
                break;
            }
        }else{
            LOG(WARNING) << "Unknown event node";
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
    // 检查所有服务发送列表，并分发消息

    while(m_cur_worker_count)
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
        CheckStopWorkers(true);
        CheckStopWorkers(false);
        // 2. 等待事件
        if(0 <= (ev_count = epoll_wait(m_epoll_fd, evs, max_ev_count, time_wait))) {
            // 3. 处理事件
            //  - 获得事件区分事件类型
            //      - 线程事件
            //      - timer事件
            //      - socket事件
            ProcessEvent(evs, ev_count);
        }else{
            LOG(ERROR) << my_get_error();
        }
        // 4. loop
    }

    // quit MyApp
    free(evs);
    close(m_epoll_fd);

    LOG(INFO) << "MyFrame Exit";
    return 0;
}
