#include "MyApp.h"

#include <errno.h>
#include <signal.h>
#include <sys/epoll.h>

#include <iostream>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>

#include "MyWorker.h"
#include "MyCUtils.h"
#include "MyContext.h"
#include "MyMsg.h"
#include "MyModule.h"
#include "MyHandleMgr.h"
#include "MySocksMgr.h"
#include "MyModules.h"
#include "MyTimerTask.h"

MyApp* MyApp::s_inst = nullptr;

MyApp::MyApp() :
    m_epoll_fd(-1),
    m_worker_count(0),
    m_quit(true),
    m_handle_mgr(nullptr),
    m_mods(nullptr),
    m_socks_mgr(nullptr)
{
    SetInherits("MyObj");
    s_inst = this;
}

MyApp::~MyApp()
{}

bool MyApp::ParseArg(int argc, char** argv)
{
    int ret = true;
    // 解析命令行输入参数
    // 命令行参数优先级高于配置文件中的参数优先级
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("conf", po::value<std::string>(), "set config file path")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help")){
        std::cout << desc << std::endl;
        return false;
    }
    std::string conf_path = "config.json";
    if(vm.count("conf")){
        conf_path = vm["conf"].as<std::string>();
    }else{
        std::cout << desc << std::endl;
        return false;
    }
    m_handle_mgr = new MyHandleMgr();
    m_mods = new MyModules();
    m_socks_mgr = new MySocksMgr();

    if(false == LoadFromConf(conf_path)) return false;
    return ret;
}

bool MyApp::LoadFromConf(std::string& filename)
{
    bool ret = true;
    namespace pt = boost::property_tree;
    pt::ptree tree;
    boost::system::error_code error;
    if(false == boost::filesystem::is_regular_file(filename, error)){
        std::cout << "Not a regular file" << std::endl;
        return false;
    }

    pt::read_json(filename, tree);
    // 获得工作线程数量
    m_worker_count_conf = tree.get("thread", 4);
    m_worker_count_conf = (m_worker_count_conf <= 0) ? 1 : m_worker_count_conf;
    m_worker_count_conf = (m_worker_count_conf >= 128) ? 128 : m_worker_count_conf;
    Start(m_worker_count_conf);
    // 获得模块路径
    m_mod_path = tree.get("module_path", ".");
    m_mods->SetModPath(m_mod_path.c_str());
    // 获得例化模块名和参数
    boost::property_tree::ptree items;
    items = tree.get_child("module_inst");
    for(boost::property_tree::ptree::iterator it=items.begin(); it != items.end(); ++it)
    {
        std::string m = it->first;
        std::string p;
        std::string s;
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v, it->second)
        {
            s = v.second.get<std::string>("name");
            p = v.second.get<std::string>("params");
            ret = CreateContext(m.c_str(), s.c_str(), p.empty() ? nullptr : p.c_str());
            m_create_mod.push_back(
                std::make_pair(m, p)
            );
        }
    }
    return ret;
}

MyApp* MyApp::Create()
{
    if(s_inst == nullptr){
        s_inst = new MyApp();
    }
    return s_inst;
}

MyApp* MyApp::Inst()
{
    return s_inst;
}

/**
 * 创建一个新的服务:
 *      1. 从MyModules中获得对应模块对象
 *      2. 生成MyContext
 *      3. 将模块对象加入MyContext对象
 *      4. 并MyContext加入Context数组
 *      3. 注册句柄
 *      4. 初始化服务
*/
bool MyApp::CreateContext(const char* mod_path, const char* mod_name, const char* service_name, const char* param)
{
    m_mods->SetModPath(mod_path);
    return CreateContext(mod_name, service_name, param);
}

bool MyApp::CreateContext(const char* mod_name, const char* service_name, const char* param)
{
    if(false == m_mods->LoadMod(mod_name)) return false;
    MyModule* mod_inst = m_mods->CreateModInst(mod_name, service_name);
    return CreateContext(mod_inst, param);
}

bool MyApp::CreateContext(MyModule* mod_inst, const char* param)
{
    MyContext* ctx = new MyContext(mod_inst);
    m_handle_mgr->RegHandle(ctx);
    ctx->Init(param);
    // 初始化之后, 手动将服务中发送消息队列分发出去
    ctx->m_recv.Append(&ctx->m_send);
    return true;
}

bool MyApp::AddEvent(MyEvent* ev)
{
    int ret = true;
    int res;
    struct epoll_event event;

    event.data.ptr = ev;
    event.events = ev->GetEpollEventType();

    BOOST_LOG_TRIVIAL(debug) << ev->GetObjName() << " reg event fd: " << ev->GetFd();
    // 如果该事件已经注册，就修改事件类型
    if(-1 == (res = epoll_ctl(m_epoll_fd,EPOLL_CTL_MOD,ev->GetFd(),&event))) {
        // 没有注册就添加至epoll
        if(-1 == (res = epoll_ctl(m_epoll_fd,EPOLL_CTL_ADD,ev->GetFd(),&event))){
            ret = false;
            BOOST_LOG_TRIVIAL(error) << "epoll " << my_get_error();
        }
    }else{
        BOOST_LOG_TRIVIAL(warning) << ev->GetObjName() << " has already reg ev " 
            << ev->GetFd() << ": " << my_get_error();
    }
    return ret;
}

bool MyApp::DelEvent(MyEvent *ev)
{
    int ret = true;
    if(-1 == epoll_ctl(m_epoll_fd,EPOLL_CTL_DEL,ev->GetFd(),NULL)){
        ret = false;
        BOOST_LOG_TRIVIAL(error) << ev->GetObjName() << " del event " 
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
        m_worker_count++;
    }
    BOOST_LOG_TRIVIAL(debug) << "Worker start";
}

void MyApp::StartTimerTask()
{
    m_timer_task = new MyTimerTask();
    m_timer_task->Start();
    AddEvent(static_cast<MyEvent*>(m_timer_task));

    BOOST_LOG_TRIVIAL(debug) << "Timer task start";
}

void MyApp::Start(int worker_count)
{
    m_epoll_fd = epoll_create(1024);
    if(-1 == m_epoll_fd){
        BOOST_LOG_TRIVIAL(error) << my_get_error();
        return;
    }
    BOOST_LOG_TRIVIAL(debug) << "Create epoll fd " << m_epoll_fd;

    StartWorker(worker_count);
    StartTimerTask();

    // ingore SIGPIPE signal
    signal(SIGPIPE,SIG_IGN);
    BOOST_LOG_TRIVIAL(debug) << "Ingore SIGPIPE signal";
    m_quit = false;
}

MyContext* MyApp::GetContext(uint32_t handle)
{
    return m_handle_mgr->GetContext(handle);
}

MySocksMgr* MyApp::GetSocksMgr()
{ return m_socks_mgr; }

// 获得一个有消息待处理的服务(m_recv不为空的服务)
MyContext* MyApp::GetContextWithMsg()
{
    // 1. 判断是否在全局数组中
    //      - 如果不是则查找下一个
    // 2. 如果在全局数组中，查看是否有消息
    //      -  有则返回，没有继续判断下一个
    // 3. 如果遍历一遍都没有就直接返回nullptr
    MyContext* ctx;
    MyContext* temp;
    ctx = temp = m_handle_mgr->GetNextContext();
    if(ctx == nullptr) return nullptr;
    for(;;){
        if(ctx->m_in_global){
            if(false == ctx->m_recv.IsEmpty()){
                return ctx;
            }
        }
        ctx = m_handle_mgr->GetNextContext();
        if(temp == ctx) break;
    }
    return nullptr;
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
    for(;begin != end;)
    {
        temp = begin->next;
        msg = static_cast<MyMsg*>(begin);
        if(msg->destination == MY_FRAME_DST){
            BOOST_LOG_TRIVIAL(debug) << "Handle MY_FRAME_DST msg";
            msg_list->Del(begin);
            m_cache_que.AddTail(begin);
        }else{
            ctx = m_handle_mgr->GetContext(msg->destination);
            if(nullptr != ctx){
                msg_list->Del(begin);
                ctx->PushMsg(begin);
            }else{
                msg_list->Del(begin);
                BOOST_LOG_TRIVIAL(error) << "Err msg src:" 
                    << msg->source << " dst:" << msg->destination 
                    << " session:" << msg->session;
            }
        }
        begin = temp;
    }
}

// 将获得的消息分发给其他服务
void MyApp::DispatchMsg(MyContext* context)
{
    if(nullptr == context) return;
    MyList* msg_list = context->GetDispatchMsgList();
    DispatchMsg(msg_list);
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
        // 主线程有消息，则先派发执行主线程中消息
        if(false == m_cache_que.IsEmpty()){
            worker->m_que.Append(&m_cache_que);
            m_idle_workers.Del(worker);
            worker->SendCmd(&cmd, sizeof(char));
            begin = temp;
            continue;
        }
        // 主线程没有消息，则派发执行服务中的消息
        if(nullptr == (context = GetContextWithMsg())) break;
        list = context->GetRecvMsgList();
        if(!list->IsEmpty()){
            worker->m_que.Append(list);
            context->m_in_global = false;
            worker->SetContext(context);
            m_idle_workers.Del(begin);
            worker->SendCmd(&cmd, sizeof(char));
        }else{
            BOOST_LOG_TRIVIAL(error) << "Context has no msg";
        }
        begin = temp;
    }
}

void MyApp::ProcessTimerEvent(MyTimerTask *timer_task)
{
    char cmd = '\0';
    timer_task->RecvCmd(&cmd, 1);
    switch(cmd){
    case 'i': // idle
        // 将定时器线程的发送队列分发完毕
        DispatchMsg(&timer_task->m_send);
        timer_task->SendCmd(&cmd, sizeof(char));
        break;
    case 'q': // quit
        // TODO...
        BOOST_LOG_TRIVIAL(warning) << "Unimplement timer task quit: " << cmd;
        break;
    default:
        BOOST_LOG_TRIVIAL(warning) << "Unknown timer task cmd: " << cmd;
        break;
    }
}

void MyApp::ProcessWorkerEvent(MyWorker* worker)
{
    char cmd = '\0';
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
    case 'q': // quit
        // TODO...
        BOOST_LOG_TRIVIAL(warning) << "Unimplement worker quit: " << cmd;
        break;
    default:
        BOOST_LOG_TRIVIAL(warning) << "Unknown worker cmd: " << cmd;
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
                BOOST_LOG_TRIVIAL(warning) << "Unknown event";
                break;
            }
        }else{
            BOOST_LOG_TRIVIAL(warning) << "Unknown event node";
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
            //      - timer事件
            //      - socket事件
            ProcessEvent(evs, ev_count);
        }else{
            BOOST_LOG_TRIVIAL(error) << my_get_error();
        }
        // 4. loop
    }

    // quit MyApp
    free(evs);
    close(m_epoll_fd);

    BOOST_LOG_TRIVIAL(info) << "MyFrame Exit";
    return 0;
}


void MyApp::Quit()
{
    // TODO...
    return;
}
