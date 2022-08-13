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
    if (!LoadModsFromConf(FLAGS_myframe_service_conf_dir)) {
        return false;
    }
    return true;
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
                LOG(INFO) << "Load actor " << *inst_name_it << " ...";
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

bool MyApp::LoadActorFromLib(
    const Json::Value& root, 
    const Json::Value& actor_list, 
    const std::string& actor_name) {
    if (!root.isMember("lib") || !root["lib"].isString()) {
        LOG(ERROR) << "actor " << actor_name <<" key \"lib\": no key or not string, skip";
        return false;
    }
    const auto& lib_name = root["lib"].asString();
    if (!_mods->LoadMod(FLAGS_myframe_lib_dir + lib_name)) {
        LOG(ERROR) << "load lib " << lib_name <<" failed, skip";
        return false;
    }
    
    const auto& insts = actor_list[actor_name];
    bool res = false;
    for (const auto& inst : insts) {
        LOG(INFO) << "create actor instance \"" << actor_name << "\": " << inst.toStyledString();
        if (!inst.isMember("instance_name")) {
            LOG(ERROR) << "actor " << actor_name <<" key \"instance_name\": no key, skip";
            return false;
        }
        if (!inst.isMember("instance_params")) {
            LOG(ERROR) << "actor " << actor_name <<" key \"instance_params\": no key, skip";
            return false;
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
            return false;
        }
        if (!inst.isMember("instance_params")) {
            LOG(ERROR) << "actor " << actor_name <<" key \"instance_params\": no key, skip";
            return false;
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
    if (!_mods->LoadMod(FLAGS_myframe_lib_dir + lib_name)) {
        LOG(ERROR) << "load lib " << lib_name <<" failed, skip";
        return false;
    }
    
    const auto& insts = worker_list[worker_name];
    bool res = false;
    for (const auto& inst : insts) {
        LOG(INFO) << "create worker instance \"" << worker_name << "\": " << inst.toStyledString();
        if (!inst.isMember("instance_name")) {
            LOG(ERROR) << "actor " << worker_name <<" key \"instance_name\": no key, skip";
            return false;
        }
        MyWorker* worker = _mods->CreateWorkerInst(lib_name, worker_name);
        worker->SetInstName("worker." + worker_name + "." + inst["instance_name"].asString());
        worker->Start();
        AddEvent(dynamic_cast<MyEvent*>(worker));
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
        worker->SetInstName("worker." + worker_name + "." + inst["instance_name"].asString());
        worker->Start();
        AddEvent(dynamic_cast<MyEvent*>(worker));
        _cur_worker_count++;
    }
    return res;
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
    mod_inst->m_instance_name = instance_name;
    return CreateContext(mod_inst, params);
}

bool MyApp::CreateContext(std::shared_ptr<MyActor>& mod_inst, const std::string& params) {
    MyContext* ctx = new MyContext(mod_inst);
    _handle_mgr->RegHandle(ctx);
    ctx->Init(params.c_str());
    // 初始化之后, 手动将actor中发送消息队列分发出去
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
        AddEvent(dynamic_cast<MyEvent*>(worker));
        _cur_worker_count++;
    }
    LOG(INFO) << "Common worker start";
}

void MyApp::StartTimerWorker() {
    _timer_worker = new MyWorkerTimer();
    _timer_worker->Start();
    AddEvent(dynamic_cast<MyEvent*>(_timer_worker));
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

    _quit = false;
}

void MyApp::DispatchMsg(std::list<std::shared_ptr<MyMsg>>& msg_list) {
    for (auto& msg : msg_list) {
        LOG(INFO) << "msg from " << msg->GetSrc() << " to " << msg->GetDst();
        auto name_list = SplitMsgName(msg->GetDst());
        if (name_list.size() < 2) {
            LOG(ERROR) << "Unknown msg dst " << msg->GetDst() << " from " << msg->GetSrc();
            continue;
        }
        // dispatch to user worker
        if (name_list[0] == "worker") {
            if (_wait_msg_workers.find(msg->GetDst()) == _wait_msg_workers.end()) {
                LOG(ERROR) << "Unknown msg src:" 
                    << msg->GetSrc() << " dst:" << msg->GetDst();
                continue;
            }
            _wait_msg_workers[msg->GetDst()]->_que.emplace_back(msg);
        }
        // dispatch to actor
        if (name_list[0] == "actor") {      
            auto ctx = _handle_mgr->GetContext(msg->GetDst());
            if(nullptr == ctx){
                LOG(ERROR) << "Unknown msg src:" 
                    << msg->GetSrc() << " dst:" << msg->GetDst();
                continue;
            }
            ctx->PushMsg(msg);
            _handle_mgr->PushContext(ctx);
        }
    }
    msg_list.clear();
}

// 将获得的消息分发给其他actor
void MyApp::DispatchMsg(MyContext* context) {
    if(nullptr == context) return;
    context->SetWaitFlag();
    auto& msg_list = context->GetDispatchMsgList();
    DispatchMsg(msg_list);
}

void MyApp::CheckStopWorkers() {
    MyContext* context;
    for (auto it = _wait_msg_workers.begin(); it != _wait_msg_workers.end();) {
        if (it->second->_que.size() > 0) {
            auto worker = it->second;
            it = _wait_msg_workers.erase(it);
            worker->SendCmdToWorker(MyWorkerCmd::RUN);
            continue;
        }
        ++it;
    }
    for (auto it = _idle_workers.begin(); it != _idle_workers.end();) {
        auto worker = *it;
        if(nullptr == (context = _handle_mgr->GetContextWithMsg())) {
            break;
        }
        auto& msg_list = context->GetRecvMsgList();
        if(!msg_list.empty()){
            MyListAppend(worker->_que, msg_list);
            worker->SetContext(context);
            it = _idle_workers.erase(it);
            worker->SendCmdToWorker(MyWorkerCmd::RUN);
            continue;
        }else{
            LOG(ERROR) << "Context has no msg";
        }
        ++it;
    }
}

void MyApp::ProcessTimerEvent(MyWorkerTimer *timer_worker) {
    // 将定时器线程的发送队列分发完毕
    DispatchMsg(timer_worker->_send);

    MyWorkerCmd cmd;
    timer_worker->RecvCmdFromWorker(cmd);
    switch(cmd){
    case MyWorkerCmd::IDLE: // idle
        timer_worker->SendCmdToWorker(MyWorkerCmd::RUN);
        break;
    case MyWorkerCmd::QUIT: // quit
        DelEvent(timer_worker);
        timer_worker->SendCmdToWorker(MyWorkerCmd::QUIT);
        _cur_worker_count--;
        LOG(WARNING) << "timer task quit: " << (char)cmd;
        break;
    default:
        LOG(WARNING) << "Unknown timer task cmd: " << (char)cmd;
        break;
    }
}

void MyApp::ProcessUserEvent(MyWorker *worker) {
    // 将用户线程的发送队列分发完毕
    DispatchMsg(worker->_send);

    MyWorkerCmd cmd;
    worker->RecvCmdFromWorker(cmd);
    switch(cmd){
    case MyWorkerCmd::IDLE: // idle
        worker->SendCmdToWorker(MyWorkerCmd::RUN);
        break;
    case MyWorkerCmd::QUIT: // quit
        DelEvent(worker);
        worker->SendCmdToWorker(MyWorkerCmd::QUIT);
        _cur_worker_count--;
        LOG(WARNING) << "user worker quit: " << (char)cmd;
        break;
    case MyWorkerCmd::WAIT_FOR_MSG:
        _wait_msg_workers[worker->GetInstName()] = worker;
        break;
    default:
        LOG(WARNING) << "Unknown user worker cmd: " << (char)cmd;
        break;
    }
}

void MyApp::ProcessWorkerEvent(MyWorkerCommon* worker) {
    // 将工作线程的发送队列分发完毕
    DispatchMsg(worker->_send);

    // 将actor的发送队列分发完毕
    DispatchMsg(worker->_context);

    MyWorkerCmd cmd;
    worker->RecvCmdFromWorker(cmd);
    switch(cmd){
    case MyWorkerCmd::IDLE: // idle
        // 将工作线程中的actor状态设置为全局状态
        // 将线程加入空闲队列
        worker->Idle();
        _idle_workers.emplace_back(worker);
        break;
    case MyWorkerCmd::QUIT: // quit    
        DelEvent(dynamic_cast<MyEvent*>(worker));
        worker->SendCmdToWorker(MyWorkerCmd::QUIT);
        _cur_worker_count--;
        LOG(WARNING) << "common worker quit: " << (char)cmd;
        break;
    default:
        LOG(WARNING) << "Unknown common worker cmd: " << (char)cmd;
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
            ProcessWorkerEvent(dynamic_cast<MyWorkerCommon*>(ev_obj));
            break;
        case MyEventType::EV_TIMER:
            // 分发超时消息
            ProcessTimerEvent(dynamic_cast<MyWorkerTimer*>(ev_obj));
            break;
        case MyEventType::EV_USER:
            ProcessUserEvent(dynamic_cast<MyWorker*>(ev_obj));
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
