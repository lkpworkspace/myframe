#include "MyWorker.h"
#include "MyLog.h"
#include "MyCUtils.h"
#include "MyContext.h"
#include "MyMsg.h"

#include <assert.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>


MyWorker::MyWorker()
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
    MYLOG(MYLL_INFO, ("The thread %d init\n", GetThreadId()));
}

void MyWorker::OnExit()
{
    // TODO...
    MYLOG(MYLL_INFO, ("The thread %d exit\n", GetThreadId()));

    MyThread::OnExit();
}

int MyWorker::Work()
{
    MyMsg* msg;
    MyEvent* event;
    MyNode* temp;
    MyNode* begin = m_que.Begin();
    MyNode* end = m_que.End();
    while(begin != end)
    {
        temp = begin->next;
        m_que.Del(begin,false);

        if(m_context){
            // 处理工作队列中的消息交
            //  1. des == 该服务的handle，由服务进行处理
            //  2. des != 该服务的handle，由系统进行处理
            msg = static_cast<MyMsg*>(begin);
            if(msg->destination == m_context->m_handle){
                m_context->CB(msg);
            }else if(msg->destination == MY_MYFRAME_MSG){
                MYLOG(MYLL_INFO, ("thread %d get a system msg\n", GetThreadId()));
                // 处理请求消息
                // 将处理后产生的消息放入m_send队列
                // TODO...
                // 此处回复一条消息给服务
                const char* re = "system msg";
                my_send(my_context(msg->source), MY_MYFRAME_MSG, msg->source, 0, 0, (void*)re, strlen(re));
            }else{
                MYLOG(MYLL_ERROR, ("thread %d get a unknown event msg\n", GetThreadId()));
            }
        }else{
            switch(begin->GetNodeType()){
            case NODE_EVENT:
                MYLOG(MYLL_INFO, ("thread %d get a event msg\n", GetThreadId()));
                event = static_cast<MyEvent*>(begin);
                switch (event->GetEventType()) {
                case EV_SOCK:
                    // for socket:
                    //      处理读写事件
                    //      处理完毕将产生的消息缓存到工作线程的发送队列
                    m_send.Append(static_cast<MyList*>(event->CB(event)));
                    break;
                default:
                    MYLOG(MYLL_ERROR, ("thread %d get a unknown event msg\n", GetThreadId()));
                    assert(false);
                    break;
                }
                break;
            default:
                MYLOG(MYLL_ERROR, ("thread %d get a unknown msg\n", GetThreadId()));
                assert(false);
                break;
            }
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
    char cmd = 'i'; // idle
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
        MYLOG(MYLL_ERROR,("sockpair failed\n"));
        return false;
    }
    ret = my_set_nonblock(m_sockpair[0], false);
    if(!ret) {
        MYLOG(MYLL_ERROR,("set sockpair nonblock failed\n"));
        return ret;
    }
    ret = my_set_nonblock(m_sockpair[1], false);
    if(!ret) {
        MYLOG(MYLL_ERROR,("set sockpair nonblock failed\n"));
        return ret;
    }
    return ret;
}

void MyWorker::CloseSockPair()
{
    if(-1 == close(m_sockpair[0])){
        MYLOG(MYLL_ERROR, ("%s\n", my_get_error()));
    }
    if(-1 == close(m_sockpair[1])){
        MYLOG(MYLL_ERROR, ("%s\n", my_get_error()));
    }
}
