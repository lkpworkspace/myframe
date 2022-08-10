#include <errno.h>
#include <signal.h>
#include <sys/epoll.h>

#include <iostream>

#include <jsoncpp/json/json.h>

#include "MyApp.h"
#include "MyCUtils.h"
#include "MyFlags.h"
#include "MyLog.h"
#include "MyMsg.h"
#include "MyActor.h"
#include "MyContext.h"
#include "MyHandleManager.h"
#include "MyModManager.h"
#include "MyWorkerCommon.h"
#include "MyWorkerTimer.h"

MyApp* MyApp::s_inst = nullptr;

MyApp* MyApp::Create() {
    if (s_inst == nullptr) {
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
    _handle_mgr = std::make_shared<MyHandleManager>();
    _mods = std::make_shared<MyModManager>();

    /// start worker
    Start(FLAGS_worker_count);

    /// load actor and worker
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
        // worker
        if (root.isMember("worker") && root["worker"].isObject()) {
            const auto& worker_list = root["worker"];
            Json::Value::Members worker_name_list = worker_list.getMemberNames();
            for (auto inst_name_it = worker_name_list.begin(); inst_name_it != worker_name_list.end(); ++inst_name_it) {
                LOG(INFO) << "Load worker " << *inst_name_it << " ...";
                if (type == "library") {
                    res |= LoadWorkerFromLib(root, worker_list, *inst_name_it);
                } else if (type == "class") {
                    res |= LoadWorkerFromClass(root, worker_list, *inst_name_it);
                } else {
                    LOG(ERROR) << "Unknown type " << type;
                }
            }
        }
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
        LOG(INFO) << "create service instance \"" << service_name << "\": " << inst.toStyledString();
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
        LOG(INFO) << "create instance \"class\"" << ": " << inst.toStyledString();
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

bool MyApp::LoadWorkerFromLib(
    const Json::Value& root, 
    const Json::Value& worker_list, 
    const std::string& worker_name) {
    if (!root.isMember("lib") || !root["lib"].isString()) {
        LOG(ERROR) << "worker \"" << worker_name << "\" key \"lib\": no key or not string, skip";
        return false;
    }
    const auto& lib_name = root["lib"].asString();
    if (!_mods->LoadMod(FLAGS_service_lib_path + lib_name)) {
        LOG(ERROR) << "load lib " << lib_name <<" failed, skip";
        return false;
    }
    
    const auto& insts = worker_list[worker_name];
    bool res = false;
    for (const auto& inst : insts) {
        LOG(INFO) << "create worker instance \"" << worker_name << "\": " << inst.toStyledString();
        if (!inst.isMember("instance_name")) {
            LOG(ERROR) << "service " << worker_name <<" key \"instance_name\": no key, skip";
            return false;
        }
        MyWorker* worker = _mods->CreateWorkerInst(lib_name, worker_name);
        worker->SetInstName(inst["instance_name"].asString());
        worker->Start();
        AddEvent(static_cast<MyEvent*>(worker));
        _cur_worker_count++;
    }
    return res;
}

bool MyApp::LoadWorkerFromClass(
    const Json::Value& root, 
    const Json::Value& worker_list, 
    const std::string& worker_name) {
    const auto& insts = worker_list[worker_name];
    bool res = true;
    for (const auto& inst : insts) {
        LOG(INFO) << "create instance \"class\"" << ": " << inst.toStyledString();
        if (!inst.isMember("instance_name")) {
            LOG(ERROR) << "worker \"" << worker_name << "\" key \"instance_name\": no key, skip";
            return false;
        }
        MyWorker* worker = _mods->CreateWorkerInst("class", worker_name);
        worker->SetInstName(inst["instance_name"].asString());
        worker->Start();
        AddEvent(static_cast<MyEvent*>(worker));
        _cur_worker_count++;
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
    auto mod_inst = _mods->CreateActorInst(mod_name, service_name);
    if(mod_inst == nullptr) {
        LOG(ERROR) << "Create mod " << mod_name << "." << service_name << " failed";
        return false;
    }
    mod_inst->m_instance_name = instance_name;
    return CreateContext(mod_inst, params);
}

bool MyApp::CreateContext(std::shared_ptr<MyActor>& mod_inst, const std::string& params) {
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
        MyWorkerCommon* worker = new MyWorkerCommon();
        worker->Start();
        AddEvent(static_cast<MyEvent*>(worker));
        _cur_worker_count++;
    }
    LOG(INFO) << "Common worker start";
}

void MyApp::StartTimerWorker() {
    _timer_worker = new MyWorkerTimer();
    _timer_worker->Start();
    AddEvent(static_cast<MyEvent*>(_timer_worker));
    LOG(INFO) << "Timer worker start";
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
    return _handle_mgr->GetContextWithMsg();
}

void MyApp::HandleSysMsg(std::shared_ptr<MyMsg>& msg) {
    LOG(INFO) << "main thread get unknown msg type: " << (int)msg->GetMsgType();
}

void MyApp::DispatchMsg(std::list<std::shared_ptr<MyMsg>>& msg_list) {
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
}

// 将获得的消息分发给其他服务
void MyApp::DispatchMsg(MyContext* context) {
    if(nullptr == context) return;
    context->SetWaitFlag();
    auto& msg_list = context->GetDispatchMsgList();
    DispatchMsg(msg_list);
}

void MyApp::CheckStopWorkers() {
    char cmd = 'y';

    MyWorkerCommon* worker;
    MyContext* context;
    MyNode* begin;
    MyNode* end;
    MyNode* temp;

    MyList& idle_workers = _idle_workers;
    begin= idle_workers.Begin();
    end = idle_workers.End();
    for(;begin != end;)
    {
        temp = begin->next;
        worker = static_cast<MyWorkerCommon*>(begin);
        if(nullptr == (context = GetContextWithMsg())) {
            LOG(INFO) << "no msg need process";
            break;
        }
        auto& msg_list = context->GetRecvMsgList();
        if(!msg_list.empty()){
            MyListAppend(worker->_que, msg_list);
            worker->SetContext(context);
            idle_workers.Del(begin);
            worker->SendCmdToWorker(MyWorkerCmd::RUN);
        }else{
            LOG(ERROR) << "Context has no msg";
        }
        begin = temp;
    }
}

void MyApp::ProcessTimerEvent(MyWorkerTimer *timer_worker) {
    MyWorkerCmd cmd;
    timer_worker->RecvCmdFromWorker(cmd);
    switch(cmd){
    case MyWorkerCmd::IDLE: // idle
        // 将定时器线程的发送队列分发完毕
        DispatchMsg(timer_worker->GetMsgList());
        timer_worker->SendCmdToWorker(MyWorkerCmd::RUN);
        break;
    case MyWorkerCmd::QUIT: // quit
        // TODO...
        LOG(WARNING) << "Unimplement timer task quit: " << (char)cmd;
        break;
    default:
        LOG(WARNING) << "Unknown timer task cmd: " << (char)cmd;
        break;
    }
}

void MyApp::ProcessUserEvent(MyWorker *worker) {
    MyWorkerCmd cmd;
    worker->RecvCmdFromWorker(cmd);
    switch(cmd){
    case MyWorkerCmd::IDLE: // idle
        // 将用户线程的发送队列分发完毕
        DispatchMsg(worker->GetMsgList());
        worker->SendCmdToWorker(MyWorkerCmd::RUN);
        break;
    case MyWorkerCmd::QUIT: // quit
        LOG(WARNING) << "Unimplement worker task quit: " << (char)cmd;
        break;
    default:
        LOG(WARNING) << "Unknown timer task cmd: " << (char)cmd;
        break;
    }
}

void MyApp::ProcessWorkerEvent(MyWorkerCommon* worker) {
    MyWorkerCmd cmd;
    worker->RecvCmdFromWorker(cmd);
    switch(cmd){
    case MyWorkerCmd::IDLE: // idle
        // 将主线程的缓存消息分发完毕
        DispatchMsg(_cache_que);

        // 将工作线程的发送队列分发完毕
        DispatchMsg(worker->GetMsgList());

        // 将服务的发送队列分发完毕
        DispatchMsg(worker->_context);

        // 将工作线程中的服务状态设置为全局状态
        // 将线程加入空闲队列
        worker->Idle();
        _idle_workers.AddTail(static_cast<MyNode*>(worker));
    
        break;
    case MyWorkerCmd::QUIT: // quit
        LOG(WARNING) << "Unimplement worker quit: " << (char)cmd;
        break;
    default:
        LOG(WARNING) << "Unknown worker cmd: " << (char)cmd;
        break;
    }
}

void MyApp::ProcessEvent(struct epoll_event* evs, int ev_count) {
    MyEvent* ev_obj;
    for(int i = 0; i < ev_count; ++i) {
        ev_obj = static_cast<MyEvent*>(evs[i].data.ptr);
        ev_obj->RetEpollEventType(evs[i].events);
        switch(ev_obj->GetMyEventType()){
        case MyEventType::EV_WORKER:
            // 工作线程事件
            ProcessWorkerEvent(static_cast<MyWorkerCommon*>(ev_obj));
            break;
        case MyEventType::EV_TIMER:
            // 分发超时消息
            ProcessTimerEvent(static_cast<MyWorkerTimer*>(ev_obj));
            break;
        case MyEventType::EV_USER:
            ProcessUserEvent(static_cast<MyWorker*>(ev_obj));
            break;
        default:
            LOG(WARNING) << "Unknown event";
            break;
        }
    }
}

int MyApp::Exec() {
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
