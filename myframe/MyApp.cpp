#include "MyApp.h"

#include <errno.h>
#include <signal.h>
#include <sys/epoll.h>

#include <iostream>

#include <jsoncpp/json/json.h>

#include "MyCUtils.h"
#include "MyFlags.h"
#include "MyLog.h"
#include "MyMsg.h"
#include "MyModule.h"
#include "MyContext.h"
#include "MyWorker.h"
#include "MyHandleMgr.h"
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
    _epoll_fd(-1),
    _cur_worker_count(0),
    _quit(true) {
    SetInherits("MyObj");
    s_inst = this;
}

MyApp::~MyApp()
{}

bool MyApp::Init() {
    _handle_mgr = std::make_shared<MyHandleMgr>();
    _mods = std::make_shared<MyModManager>();

    /// start worker
    Start(FLAGS_worker_count);

    /// load modules
    if (!LoadModsFromConf(FLAGS_service_desc_path)) {
        return false;
    }
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

bool MyApp::CreateContext(std::shared_ptr<MyModule>& mod_inst, const std::string& params) {
    MyContext* ctx = new MyContext(mod_inst);
    _handle_mgr->RegHandle(ctx);
    ctx->Init(params.c_str());
    // 初始化之后, 手动将服务中发送消息队列分发出去
    DispatchMsg(ctx);
    return true;
}

bool MyApp::AddEvent(MyEvent* ev) {
    struct epoll_event event;
    event.data.ptr = ev;
    event.events = ev->ListenEpollEventType();
    int res = 0;
    // 如果该事件已经注册，就修改事件类型
    if(-1 == (res = epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, ev->GetFd(), &event))) {
        // 没有注册就添加至epoll
        if(-1 == (res = epoll_ctl(_epoll_fd,EPOLL_CTL_ADD,ev->GetFd(),&event))){
            LOG(ERROR) << "epoll " << my_get_error();
            return false;
        }
    }else{
        LOG(WARNING) << " has already reg ev " 
            << ev->GetFd() << ": " << my_get_error();
        return false;
    }
    return true;
}

bool MyApp::DelEvent(MyEvent *ev) {
    if(-1 == epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, ev->GetFd(), NULL)){
        LOG(ERROR) << "del event " 
            << ev->GetFd() << ": " << my_get_error();
        return false;
    }
    return true;
}

void MyApp::StartCommonWorker(int worker_count) {
    for(int i = 0; i < worker_count; ++i){
        MyWorker* worker = new MyWorker();
        worker->Start();
        AddEvent(static_cast<MyEvent*>(worker));
        _cur_worker_count++;
    }
    LOG(INFO) << "Common worker start";
}

void MyApp::StartTimerWorker() {
    // m_timer_task = new MyTimerTask();
    // m_timer_task->Start();
    // AddEvent(static_cast<MyEvent*>(m_timer_task));
    // LOG(INFO) << "Timer worker start";
}

void MyApp::Start(int worker_count) {
    _epoll_fd = epoll_create(1024);
    if(-1 == _epoll_fd){
        LOG(ERROR) << my_get_error();
        return;
    }
    LOG(INFO) << "Create epoll fd " << _epoll_fd;

    StartCommonWorker(worker_count);
    StartTimerWorker();

    // ingore SIGPIPE signal
    signal(SIGPIPE,SIG_IGN);
    LOG(INFO) << "Ingore SIGPIPE signal";
    _quit = false;
}

MyContext* MyApp::GetContext(uint32_t handle) {
    return _handle_mgr->GetContext(handle);
}

MyContext* MyApp::GetContext(std::string& service_name) {
    return _handle_mgr->GetContext(service_name);
}

MyContext* MyApp::GetContextWithMsg() {
    return _handle_mgr->GetContext();
}

void MyApp::HandleSysMsg(std::shared_ptr<MyMsg>& msg) {
    LOG(INFO) << "main thread get unknown msg type: " << (int)msg->GetMsgType();
}

void MyApp::DispatchMsg(std::list<std::shared_ptr<MyMsg>>& msg_list) {
    m_mutex.lock();
    for (auto& msg : msg_list) {
        if(msg->destination == MY_FRAME_DST){
            HandleSysMsg(msg);
            continue;
        }
        auto ctx = _handle_mgr->GetContext(msg->destination);
        if(nullptr == ctx){
            LOG(ERROR) << "msg src:" 
                << msg->source << " dst:" << msg->destination 
                << " session:" << msg->session;
            continue;
        }
        ctx->PushMsg(msg);
        _handle_mgr->PushContext(ctx);
    }
    msg_list.clear();
    m_mutex.unlock();
}

// 将获得的消息分发给其他服务
void MyApp::DispatchMsg(MyContext* context) {
    if(nullptr == context) return;
    context->m_in_global = true;
    auto& msg_list = context->GetDispatchMsgList();
    DispatchMsg(msg_list);
}

void MyApp::CheckStopWorkers() {
    char cmd = 'y';

    MyWorker* worker;
    MyContext* context;
    MyNode* begin;
    MyNode* end;
    MyNode* temp;

    MyList& idle_workers = _idle_workers;
    begin= idle_workers.Begin();
    end = idle_workers.End();
    m_mutex.lock();
    for(;begin != end;)
    {
        temp = begin->next;
        worker = static_cast<MyWorker*>(begin);
        if(nullptr == (context = GetContextWithMsg())) {
            LOG(INFO) << "no msg need process";
            break;
        }
        auto& msg_list = context->GetRecvMsgList();
        if(!msg_list.empty()){
            MyListAppend(worker->_que, msg_list);
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
    // char cmd = 'y';
    // timer_task->RecvCmd(&cmd, 1);
    // switch(cmd){
    // case 'i': // idle
    //     // 将定时器线程的发送队列分发完毕
    //     DispatchMsg(timer_task->_send);
    //     timer_task->SendCmd(&cmd, sizeof(char));
    //     break;
    // case 'q': // quit
    //     // TODO...
    //     LOG(WARNING) << "Unimplement timer task quit: " << cmd;
    //     break;
    // default:
    //     LOG(WARNING) << "Unknown timer task cmd: " << cmd;
    //     break;
    // }
}

void MyApp::ProcessWorkerEvent(MyWorker* worker)
{
    char cmd = 'y';
    worker->RecvCmd(&cmd, 1);
    switch(cmd){
    case 'i': // idle
        // 将主线程的缓存消息分发完毕
        DispatchMsg(_cache_que);

        // 将工作线程的发送队列分发完毕
        DispatchMsg(worker->_send);

        // 将服务的发送队列分发完毕
        DispatchMsg(worker->_context);

        // 将工作线程中的服务状态设置为全局状态
        // 将线程加入空闲队列
        worker->Idle();
        _idle_workers.AddTail(static_cast<MyNode*>(worker));
    
        break;
    case 'q': // quit
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
        ev_obj->RetEpollEventType(evs[i].events);
        if(MyNode::NODE_EVENT == ev_obj->GetNodeType()){
            switch(ev_obj->GetEventType()){
            case MyEvent::EV_WORKER:
                // 工作线程事件
                ProcessWorkerEvent(static_cast<MyWorker*>(ev_obj));break;
            case MyEvent::EV_TIMER:
                // 分发超时消息
                ProcessTimerEvent(static_cast<MyTimerTask*>(ev_obj));
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
    
    while(_cur_worker_count) {
        /// 检查空闲线程队列是否有空闲线程，如果有就找到一个有消息的服务处理
        CheckStopWorkers();
        /// 等待事件
        if(0 > (ev_count = epoll_wait(_epoll_fd, evs, max_ev_count, time_wait))) {
            LOG(ERROR) << "epoll wait error: " << my_get_error();
        }
        /// 处理事件
        ProcessEvent(evs, ev_count);
    }

    // quit MyApp
    free(evs);
    close(_epoll_fd);

    LOG(INFO) << "myframe exit";
    return 0;
}
