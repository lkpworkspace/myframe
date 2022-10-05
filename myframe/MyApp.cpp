/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include <errno.h>
#include <signal.h>
#include <sys/epoll.h>

#include <iostream>

#include "MyCommon.h"
#include "MyApp.h"
#include "MyCUtils.h"
#include "MyFlags.h"
#include "MyLog.h"
#include "MyMsg.h"
#include "MyActor.h"
#include "MyContext.h"
#include "MyContextManager.h"
#include "MyModManager.h"
#include "MyWorkerManager.h"
#include "MyWorkerCommon.h"
#include "MyWorkerTimer.h"

namespace myframe {

std::shared_ptr<MyWorkerTimer> MyApp::GetTimerWorker() { 
    auto w = _worker_mgr->Get(myframe::FLAGS_worker_timer_name);
    return std::dynamic_pointer_cast<MyWorkerTimer>(w);
}

MyApp::MyApp() 
    : _epoll_fd(-1)
    , _context_mgr(new MyContextManager())
    , _mods(new MyModManager())
    , _worker_mgr(new MyWorkerManager())
{}

MyApp::~MyApp()
{}

bool MyApp::Init() {
    _epoll_fd = epoll_create(1024);
    if(-1 == _epoll_fd){
        LOG(ERROR) << my_get_error();
        return false;
    }
    LOG(INFO) << "Create epoll fd " << _epoll_fd;

    bool ret = true;
    ret &= StartCommonWorker(myframe::FLAGS_worker_count);
    ret &= StartTimerWorker();

    _quit = false;
    return ret;
}

bool MyApp::LoadModsFromConf(const std::string& path) {
    auto actor_conf_list = MyCommon::GetDirFiles(path);
    LOG(INFO) << "Search " << actor_conf_list.size() << " actor conf"
              << ", from " << path;
    if (actor_conf_list.size() <= 0) {
        LOG(WARNING) << "Search actor failed, exit";
        return false;
    }
    bool res = false;
    for (const auto& it : actor_conf_list) {
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
        // load actor
        if (root.isMember("actor") && root["actor"].isObject()) {
            const auto& actor_list = root["actor"];
            Json::Value::Members actor_name_list = actor_list.getMemberNames();
            for (auto inst_name_it = actor_name_list.begin(); inst_name_it != actor_name_list.end(); ++inst_name_it) {
                LOG(INFO) << "search actor " << *inst_name_it << " ...";
                if (type == "library") {
                    res |= LoadActorFromLib(root, actor_list, *inst_name_it);
                } else if (type == "class") {
                    res |= LoadActorFromClass(root, actor_list, *inst_name_it);
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
                LOG(INFO) << "search worker " << *inst_name_it << " ...";
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

bool MyApp::LoadActorFromLib(
    const Json::Value& root, 
    const Json::Value& actor_list, 
    const std::string& actor_name) {
    if (!root.isMember("lib") || !root["lib"].isString()) {
        LOG(ERROR) << "actor " << actor_name <<" key \"lib\": no key or not string, skip";
        return false;
    }
    const auto& lib_name = root["lib"].asString();
    if (!_mods->LoadMod(myframe::FLAGS_myframe_lib_dir + lib_name)) {
        LOG(ERROR) << "load lib " << lib_name <<" failed, skip";
        return false;
    }
    
    const auto& insts = actor_list[actor_name];
    bool res = false;
    for (const auto& inst : insts) {
        LOG(INFO) << "create actor instance \"" << actor_name << "\": " << inst.toStyledString();
        if (!inst.isMember("instance_name")) {
            LOG(ERROR) << "actor " << actor_name <<" key \"instance_name\": no key, skip";
            continue;
        }
        if (!inst.isMember("instance_params")) {
            LOG(ERROR) << "actor " << actor_name <<" key \"instance_params\": no key, skip";
            continue;
        }
        res |= CreateContext(
            lib_name, actor_name, 
            inst["instance_name"].asString(), 
            inst["instance_params"].asString());
    }
    return res;
}

bool MyApp::LoadActorFromClass(
    const Json::Value& root, 
    const Json::Value& actor_list, 
    const std::string& actor_name) {
    const auto& insts = actor_list[actor_name];
    bool res = false;
    for (const auto& inst : insts) {
        LOG(INFO) << "create instance \"class\"" << ": " << inst.toStyledString();
        if (!inst.isMember("instance_name")) {
            LOG(ERROR) << "actor " << actor_name <<" key \"instance_name\": no key, skip";
            continue;
        }
        if (!inst.isMember("instance_params")) {
            LOG(ERROR) << "actor " << actor_name <<" key \"instance_params\": no key, skip";
            continue;
        }
        res |= CreateContext(
            "class", actor_name, 
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
    if (!_mods->LoadMod(myframe::FLAGS_myframe_lib_dir + lib_name)) {
        LOG(ERROR) << "load lib " << lib_name <<" failed, skip";
        return false;
    }
    
    const auto& insts = worker_list[worker_name];
    bool res = false;
    for (const auto& inst : insts) {
        LOG(INFO) << "create worker instance \"" << worker_name << "\": " << inst.toStyledString();
        if (!inst.isMember("instance_name")) {
            LOG(ERROR) << "worker " << worker_name <<" key \"instance_name\": no key, skip";
            continue;
        }
        res = true;
        auto worker = _mods->CreateWorkerInst(lib_name, worker_name);
        if (worker == nullptr) {
            LOG(ERROR) << "create worker " << lib_name
                << "." << worker_name << "." << inst["instance_name"].asString()
                << " failed, continue";
            continue;
        }
        AddWorker(inst["instance_name"].asString(), worker);
    }
    return res;
}

bool MyApp::LoadWorkerFromClass(
    const Json::Value& root, 
    const Json::Value& worker_list, 
    const std::string& worker_name) {
    const auto& insts = worker_list[worker_name];
    bool res = false;
    for (const auto& inst : insts) {
        LOG(INFO) << "create instance \"class\"" << ": " << inst.toStyledString();
        if (!inst.isMember("instance_name")) {
            LOG(ERROR) << "worker \"" << worker_name << "\" key \"instance_name\": no key, skip";
            continue;
        }
        res = true;
        auto worker = _mods->CreateWorkerInst("class", worker_name);
        if (worker == nullptr) {
            LOG(ERROR) << "create worker "
                << "class." << worker_name << "." << inst["instance_name"].asString()
                << " failed, continue";
            continue;
        }
        AddWorker(inst["instance_name"].asString(), worker);
    }
    return res;
}

bool MyApp::AddActor(
    const std::string& inst_name, 
    const std::string& params, 
    std::shared_ptr<MyActor> actor) {
    actor->SetInstName(inst_name);
    return CreateContext(actor, params);
}

bool MyApp::AddWorker(
    const std::string& inst_name, 
    std::shared_ptr<MyWorker> worker) {
    worker->SetInstName(inst_name);
    if (!_worker_mgr->Add(worker)) {
        return false;
    }
    if (!AddEvent(std::dynamic_pointer_cast<MyEvent>(worker))) {
        return false;
    }
    worker->Start();
    return true;
}

/**
 * 创建一个新的actor:
 *      1. 从MyModManager中获得对应模块对象
 *      2. 生成MyContext
 *      3. 将模块对象加入MyContext对象
 *      4. 将MyContext加入Context数组
 *      3. 注册句柄
 *      4. 初始化actor
*/
bool MyApp::CreateContext(
    const std::string& mod_name, 
    const std::string& actor_name, 
    const std::string& instance_name, 
    const std::string& params) {
    auto mod_inst = _mods->CreateActorInst(mod_name, actor_name);
    if(mod_inst == nullptr) {
        LOG(ERROR) << "Create mod " << mod_name << "." << actor_name << " failed";
        return false;
    }
    mod_inst->SetInstName(instance_name);
    return CreateContext(mod_inst, params);
}

bool MyApp::CreateContext(std::shared_ptr<MyActor>& mod_inst, const std::string& params) {
    auto ctx = std::make_shared<MyContext>(shared_from_this(), mod_inst);
    ctx->Init(params.c_str());
    _context_mgr->RegContext(ctx);
    // 初始化之后, 手动将actor中发送消息队列分发出去
    DispatchMsg(ctx);
    return true;
}

bool MyApp::AddEvent(std::shared_ptr<MyEvent> ev) {
    struct epoll_event event;
    event.data.fd = ev->GetFd();
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

bool MyApp::DelEvent(std::shared_ptr<MyEvent> ev) {
    if(-1 == epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, ev->GetFd(), NULL)){
        LOG(ERROR) << "del event " 
            << ev->GetFd() << ": " << my_get_error();
        return false;
    }
    return true;
}

bool MyApp::StartCommonWorker(int worker_count) {
    bool ret = false;
    for(int i = 0; i < worker_count; ++i){
        auto worker = std::make_shared<MyWorkerCommon>();
        worker->SetModName("class");
        worker->SetTypeName("MyWorkerCommon");
        if (!AddWorker(std::to_string(i), worker)) {
            LOG(ERROR) << "start common worker " << i << " failed";
            continue;
        }
        LOG(INFO) << "start common worker " << worker->GetWorkerName();
        ret = true;
    }
    return ret;
}

bool MyApp::StartTimerWorker() {
    auto worker = std::make_shared<MyWorkerTimer>();
    worker->SetModName("class");
    worker->SetTypeName("timer");
    if (!AddWorker("#1", worker)) {
        LOG(ERROR) << "start timer worker failed";
        return false;
    }
    LOG(INFO) << "start timer worker " << worker->GetWorkerName();
    return true;
}

void MyApp::DispatchMsg(std::list<std::shared_ptr<MyMsg>>& msg_list) {
    LOG_IF(WARNING, msg_list.size() > myframe::FLAGS_dispatch_or_process_msg_max) << " dispatch msg too many";
    std::lock_guard<std::mutex> lock(_dispatch_mtx);
    for (auto& msg : msg_list) {
        DLOG(INFO) << "msg from " << msg->GetSrc() << " to " << msg->GetDst();
        auto name_list = SplitMsgName(msg->GetDst());
        if (name_list.size() < 2) {
            LOG(ERROR) << "Unknown msg dst " << msg->GetDst() << " from " << msg->GetSrc();
            continue;
        }
        
        if (name_list[0] == "worker") { 
            // dispatch to user worker
            _worker_mgr->DispatchWorkerMsg(msg);
        } else if (name_list[0] == "actor") {  
            // dispatch to actor    
            auto ctx = _context_mgr->GetContext(msg->GetDst());
            if(nullptr == ctx){
                LOG(ERROR) << "Unknown msg from " 
                    << msg->GetSrc() << " to " << msg->GetDst();
                continue;
            }
            ctx->PushMsg(msg);
            _context_mgr->PushContext(ctx);
        } else {
            LOG(ERROR) \
                << "Unknown msg from " 
                << msg->GetSrc() << " to " << msg->GetDst();
        }
    }
    msg_list.clear();
}

// 将获得的消息分发给其他actor
void MyApp::DispatchMsg(std::shared_ptr<MyContext> context) {
    if(nullptr == context) {
        return;
    }
    DLOG(INFO) << context->GetModule()->GetActorName() << " dispatch msg...";
    context->SetRuningFlag(false);
    auto& msg_list = context->GetDispatchMsgList();
    DispatchMsg(msg_list);
}

void MyApp::CheckStopWorkers() {
    DLOG(INFO) << "check stop worker";
    _worker_mgr->WeakupWorker();

    LOG_IF(INFO, _worker_mgr->IdleWorkerSize() == 0) << "worker busy, wait for idle worker...";
    std::shared_ptr<MyContext> context = nullptr;
    std::shared_ptr<MyWorker> worker = nullptr;
    while((worker = _worker_mgr->FrontIdleWorker()) != nullptr) {
        if(nullptr == (context = _context_mgr->GetContextWithMsg())) {
            DLOG(INFO) << "no actor need process, waiting...";
            break;
        }
        DLOG(INFO) \
            << worker->GetWorkerName() 
            << "."
            << (unsigned long)worker->GetPosixThreadId() 
            << " dispatch task to idle worker";
        auto& msg_list = context->GetRecvMsgList();
        if(!msg_list.empty()){
            LOG_IF(WARNING, msg_list.size() > myframe::FLAGS_dispatch_or_process_msg_max) \
                << context->GetModule()->GetActorName() 
                << " recv msg size too many: " << msg_list.size();
            DLOG(INFO) << "run " << context->GetModule()->GetActorName();
            MyListAppend(worker->_que, msg_list);
            _worker_mgr->PopFrontIdleWorker();
            auto common_idle_worker = std::dynamic_pointer_cast<MyWorkerCommon>(worker);
            common_idle_worker->SetContext(context);
            worker->SendCmdToWorker(MyWorkerCmd::RUN);
            continue;
        }else{
            LOG(ERROR) << context->GetModule()->GetActorName() << " has no msg";
        }
    }
}

void MyApp::ProcessTimerEvent(std::shared_ptr<MyWorkerTimer> timer_worker) {
    // 将定时器线程的发送队列分发完毕
    DLOG(INFO) << timer_worker->GetWorkerName() << " dispatch msg...";
    DispatchMsg(timer_worker->_send);

    MyWorkerCmd cmd;
    timer_worker->RecvCmdFromWorker(cmd);
    switch(cmd){
    case MyWorkerCmd::IDLE: // idle
        DLOG(INFO) << timer_worker->GetWorkerName() << " run again";
        timer_worker->SendCmdToWorker(MyWorkerCmd::RUN);
        break;
    case MyWorkerCmd::QUIT: // quit
        LOG(INFO) << timer_worker->GetWorkerName() << " quit, delete from myframe";
        DelEvent(timer_worker);
        _worker_mgr->Del(timer_worker);
        break;
    default:
        LOG(WARNING) << "Unknown timer worker cmd: " << (char)cmd;
        break;
    }
}

void MyApp::ProcessUserEvent(std::shared_ptr<MyWorker> worker) {
    // 将用户线程的发送队列分发完毕
    DLOG(INFO) << worker->GetWorkerName() << " dispatch msg...";
    DispatchMsg(worker->_send);

    MyWorkerCmd cmd;
    worker->RecvCmdFromWorker(cmd);
    switch(cmd){
    case MyWorkerCmd::IDLE: // idle
        DLOG(INFO) << worker->GetWorkerName() << " run again";
        worker->SendCmdToWorker(MyWorkerCmd::RUN);
        break;
    case MyWorkerCmd::WAIT_FOR_MSG:
        DLOG(INFO) << worker->GetWorkerName() << " wait for msg...";
        _worker_mgr->PushWaitWorker(worker);
        break;
    case MyWorkerCmd::QUIT: // quit
        LOG(INFO) << worker->GetWorkerName() << " quit, delete from myframe";
        DelEvent(worker);
        _worker_mgr->Del(worker);
        break;
    default:
        LOG(WARNING) << "Unknown user worker cmd: " << (char)cmd;
        break;
    }
}

/// FIXME: Idle/DispatchMsg 会影响actor的执行顺序
void MyApp::ProcessWorkerEvent(std::shared_ptr<MyWorkerCommon> worker) {
    // 将actor的发送队列分发完毕
    DLOG_IF(INFO, worker->GetContext() != nullptr) \
        << worker->GetWorkerName() 
        << "."
        << (unsigned long)worker->GetPosixThreadId() 
        << " dispatch "
        << worker->GetContext()->GetModule()->GetActorName()
        << " msg...";
    LOG_IF(WARNING, worker->GetContext() == nullptr) \
        << worker->GetWorkerName() 
        << "."
        << (unsigned long)worker->GetPosixThreadId() 
        << " no context";
    DispatchMsg(worker->GetContext());

    MyWorkerCmd cmd;
    worker->RecvCmdFromWorker(cmd);
    switch(cmd){
    case MyWorkerCmd::IDLE: // idle
        // 将工作线程中的actor状态设置为全局状态
        // 将线程加入空闲队列
        DLOG(INFO) \
            << worker->GetWorkerName() 
            << "."
            << (unsigned long)worker->GetPosixThreadId() 
            << " idle, push to idle queue";
        worker->Idle();
        _worker_mgr->PushBackIdleWorker(worker);
        break;
    case MyWorkerCmd::QUIT: // quit  
        LOG(INFO) \
            << worker->GetWorkerName() 
            << "."
            << (unsigned long)worker->GetPosixThreadId() 
            << " quit, delete from myframe";
        DelEvent(std::dynamic_pointer_cast<MyEvent>(worker));
        _worker_mgr->Del(worker);
        // FIXME: 应该将worker加入删除队列，等worker运行结束后再从队列删除
        // 否则会造成删除智能指针后，worker还没结束运行造成coredump
        break;
    default:
        LOG(WARNING) << "unknown common worker cmd: " << (char)cmd;
        break;
    }
}

void MyApp::ProcessEvent(struct epoll_event* evs, int ev_count) {
    DLOG_IF(INFO, ev_count > 0) << "get " << ev_count << " event";
    for(int i = 0; i < ev_count; ++i) {
        auto ev_obj = _worker_mgr->Get(evs[i].data.fd);
        ev_obj->RetEpollEventType(evs[i].events);
        switch(ev_obj->GetMyEventType()){
        case MyEventType::WORKER_COMMON:
            ProcessWorkerEvent(std::dynamic_pointer_cast<MyWorkerCommon>(ev_obj));
            break;
        case MyEventType::WORKER_TIMER:
            ProcessTimerEvent(std::dynamic_pointer_cast<MyWorkerTimer>(ev_obj));
            break;
        case MyEventType::WORKER_USER:
            ProcessUserEvent(ev_obj);
            break;
        default:
            LOG(WARNING) << "unknown event";
            break;
        }
    }
}

int MyApp::Exec() {
    int ev_count = 0;
    int max_ev_count = 64;
    int time_wait = -1;
    struct epoll_event* evs;
    evs = (struct epoll_event*)malloc(sizeof(struct epoll_event) * max_ev_count);
    
    while(_worker_mgr->WorkerSize()) {
        /// 检查空闲线程队列是否有空闲线程，如果有就找到一个有消息的actor处理
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

} // namespace myframe