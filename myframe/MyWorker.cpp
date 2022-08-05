#include "MyWorker.h"

#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "MyLog.h"
#include "MyCUtils.h"
#include "MyContext.h"
#include "MyMsg.h"
#include "MyApp.h"
#include "MySocksMgr.h"
#include "MyHandleMgr.h"


MyWorker::MyWorker() :
    m_context(nullptr),
    m_cmd(MyWorkerCmdType::IDLE)
{
    SetInherits("MyThread");
    CreateSockPair();
}

MyWorker::~MyWorker()
{
    CloseSockPair();
}

void MyWorker::Idle()
{
    if(m_context){
        m_context->m_in_global = true;
        m_context = nullptr;
    }
}

void MyWorker::Run()
{
    Wait();
    Work();
}

void MyWorker::OnInit()
{
    MyThread::OnInit();

    // TODO...
    LOG(INFO) << "Worker " << GetThreadId() << " init";
}

void MyWorker::OnExit()
{
    // TODO...
    LOG(INFO) << "Worker " << GetThreadId() << " exit";

    MyThread::OnExit();
}

int MyWorker::Work()
{
    MyContext* ctx;
    MyMsg* msg;
    MyEvent* event;
    MyNode* temp;
    MyNode* begin = m_que.Begin();
    MyNode* end = m_que.End();

    while(begin != end)
    {
        temp = begin->next;
        m_que.Del(begin,false);

        if(MyNode::NODE_MSG == begin->GetNodeType()){
            msg = static_cast<MyMsg*>(begin);
            ctx = m_context;
            if(ctx){
                ctx->CB(msg);
                LOG(INFO) << "Worker: " << GetThreadId() << " get cmd: "
                    << (char)m_cmd;
            }else{
                LOG(ERROR) << "Worker " << GetThreadId() << " get a unknown msg"
                    << "src:" << msg->source << " dst:" << msg->destination;
            }
        }else if(MyNode::NODE_EVENT == begin->GetNodeType()){
            event = static_cast<MyEvent*>(begin);
            LOG(INFO) << "Worker " << GetThreadId() 
                << " get msg ev-type: " << event->GetEventType();
            switch (event->GetEventType()) {
            case EV_SOCK:{
                // for socket:
                //      判读是新连接/读事件/写事件
                //      处理读写事件
                //      处理完毕将产生的消息缓存到工作线程的发送队列
                int add;
                m_send.Append(event->CB(event, &add));
                if(add){
                    MyApp::Inst()->AddEvent(event);
                }
                break;
            }
            default:
                LOG(ERROR) << "Worker " << GetThreadId() 
                    << " get unknown msg ev-type" << event->GetEventType();
                exit(-1);
                break;
            }
        }else{
            LOG(ERROR) << "Worker " << GetThreadId() << " get unknown msg";
        }
        begin = temp;
    }
    return 0;
}

int MyWorker::GetFd()
{
    return m_sockpair[1];
}

unsigned int MyWorker::GetEpollEventType()
{
    return EPOLLIN;
}

int MyWorker::SendCmd(const char* cmd, size_t len)
{
    return write(m_sockpair[1], cmd, len);
}

int MyWorker::RecvCmd(char* cmd, size_t len)
{
    return read(m_sockpair[1], cmd, len);
}

int MyWorker::Wait()
{
    // tell MyApp, add this worker to idle worker list
    char cmd = (char)m_cmd; // idle
    write(m_sockpair[0], &cmd, 1);

    // 等待主线程唤醒工作
    return read(m_sockpair[0], &cmd, 1);
}

bool MyWorker::CreateSockPair()
{
    int res = -1;
    bool ret = true;

    res = socketpair(AF_UNIX,SOCK_DGRAM,0,m_sockpair);
    if(res == -1) {
        LOG(ERROR) << "Worker create sockpair failed";
        return false;
    }
    ret = my_set_nonblock(m_sockpair[0], false);
    if(!ret) {
        LOG(ERROR) << "Worker set sockpair[0] block failed";
        return ret;
    }
    ret = my_set_nonblock(m_sockpair[1], false);
    if(!ret) {
        LOG(ERROR) << "Worker set sockpair[1] block failed";
        return ret;
    }
    return ret;
}

void MyWorker::CloseSockPair()
{
    if(-1 == close(m_sockpair[0])){
        LOG(ERROR) << "Worker close sockpair[0]: " << my_get_error();
    }
    if(-1 == close(m_sockpair[1])){
        LOG(ERROR) << "Worker close sockpair[1]: " << my_get_error();
    }
}
