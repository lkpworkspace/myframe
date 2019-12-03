#include "MyTimerTask.h"

#include <sys/types.h>
#include <sys/socket.h>

#include <boost/log/trivial.hpp>

#include "MyCUtils.h"
#include "MyApp.h"
#include "MyMsg.h"
#include "MyModule.h"

MyTimerMgr::MyTimerMgr()
{
    m_tv[0] = m_tv2;
    m_tv[1] = m_tv3;
    m_tv[2] = m_tv4;
    m_tv[3] = m_tv5;

    m_cur_point = my_gettime_ms() / 10;
}

MyTimerMgr::~MyTimerMgr()
{
}

void MyTimerMgr::_AddTimerNode(MyTimer* node)
{
    uint32_t time = node->m_expire;
    uint32_t cur_time = m_time;

    if((time | TVR_MASK) == (cur_time | TVR_MASK)){
        m_tv1[time&TVR_MASK].AddTail(node);
    }else{
        int i;
        uint32_t mask = TVR_SIZE << TVN_BITS;
        for(i = 0; i < 3; ++i){
            if ((time|(mask-1))==(cur_time|(mask-1))) {
                break;
            }
            mask <<= TVN_BITS;
        }
        m_tv[i][((time >>(TVR_BITS + i*TVN_BITS)) & TVN_MASK)].AddTail(node);
    }
}

int MyTimerMgr::Timeout(uint32_t handle, int time, int session)
{
    if(time <= 0) return -1;
    MyTimer* timer = new MyTimer();
    timer->m_handle = handle;
    timer->m_session = session;

    // add node
    m_mutex.lock();
    timer->m_expire = time + m_time;
    _AddTimerNode(timer);
    m_mutex.unlock();

    return session;
}

void MyTimerMgr::_Dispath(MyList* cur)
{
    MyRespMsg* msg;
    MyTimer* timer;
    MyNode* begin;
    MyNode* end;
    MyNode* temp;

    begin= cur->Begin();
    end = cur->End();
    for(;begin != end;)
    {
        temp = begin->next;
        cur->Del(begin);
        timer = static_cast<MyTimer*>(begin);
        msg = new MyRespMsg();
        msg->source = MY_FRAME_DST;
        msg->destination = timer->m_handle;
        msg->session = timer->m_session;
        msg->SetRespMsgType(MyRespMsg::MyRespMsgType::TIMER);
        delete begin;
        m_timeout.AddTail(msg);
        begin = temp;
    }
}

void MyTimerMgr::_Execute()
{
    int idx = m_time & TVR_MASK;
    while(!m_tv1[idx].IsEmpty()){
        _Dispath(&m_tv1[idx]);
    }
}
void MyTimerMgr::_MoveList(int level, int idx)
{
    MyList* cur = &m_tv[level][idx];
    MyTimer* timer;
    MyNode* begin;
    MyNode* end;
    MyNode* temp;

    begin= cur->Begin();
    end = cur->End();
    for(;begin != end;)
    {
        temp = begin->next;
        cur->Del(begin);
        timer = static_cast<MyTimer*>(begin);
        _AddTimerNode(timer);
        begin = temp;
    }
}
void MyTimerMgr::_Shift()
{
    int mask = TVR_SIZE;
    uint32_t ct = ++m_time;
    if (ct == 0) {
        _MoveList(3, 0);
    } else {
        uint32_t time = ct >> TVR_BITS;
        int i=0;
        // 每255个滴答需要重新分配定时器所在区间
        while ((ct & (mask-1))==0) {
            int idx=time & TVN_MASK;
            if (idx!=0) {
                _MoveList(i, idx);
                break;
            }
            mask <<= TVN_BITS;
            time >>= TVN_BITS;
            ++i;
        }
    }
}
void MyTimerMgr::_Updatetime()
{
    m_mutex.lock();
    _Execute();
    _Shift();
    _Execute();
    m_mutex.unlock();
}
MyList* MyTimerMgr::Updatetime()
{
    uint64_t cp = my_gettime_ms() / MY_RESOLUTION_MS;
    if(cp < m_cur_point){
        BOOST_LOG_TRIVIAL(error) << "Future time: " << cp << ":" << m_cur_point;
        m_cur_point = cp;
    }else if(cp != m_cur_point){
        uint32_t diff = (uint32_t)(cp - m_cur_point);
        m_cur_point = cp;
        int i;
        for(i =0; i < diff; ++i){
            _Updatetime();
        }
    }
    return &m_timeout;
}

//////////////////////////////////////////////////////

MyTimerTask::MyTimerTask()
{
    SetInherits("MyThread");
    CreateSockPair();
}

MyTimerTask::~MyTimerTask()
{
    CloseSockPair();
}

void MyTimerTask::Run()
{
    int wait = 0;
    wait = Work();
    if(wait)
        Wait();
    usleep(2500);
}

void MyTimerTask::OnInit()
{
    MyThread::OnInit();
    BOOST_LOG_TRIVIAL(debug) << "Timer task " << GetThreadId() << " init";
}

void MyTimerTask::OnExit()
{
    BOOST_LOG_TRIVIAL(debug) << "Timer task " << GetThreadId() << " exit";

    MyThread::OnExit();
}
int MyTimerTask::SetTimeout(uint32_t handle, int time, int session)
{
    return m_timer_mgr.Timeout(handle, time, session);
}

int MyTimerTask::Work()
{
    MyList* timeout;
    timeout = m_timer_mgr.Updatetime();
    m_send.Append(timeout);
    return (m_send.IsEmpty() == false) ? 1 : 0;
}

int MyTimerTask::SendCmd(const char* cmd, size_t len)
{
    return write(m_sockpair[1], cmd, len);
}

int MyTimerTask::RecvCmd(char* cmd, size_t len)
{
    return read(m_sockpair[1], cmd, len);
}

int MyTimerTask::Wait()
{
    // tell MyApp, dispatch timeout msg to service
    char cmd = 'i';
    write(m_sockpair[0], &cmd, 1);

    // 等待主线程唤醒工作
    return read(m_sockpair[0], &cmd, 1);
}

bool MyTimerTask::CreateSockPair()
{
    int res = -1;
    bool ret = true;

    res = socketpair(AF_UNIX,SOCK_DGRAM,0,m_sockpair);
    if(res == -1) {
        BOOST_LOG_TRIVIAL(error) << "Timer create sockpair failed";
        return false;
    }
    ret = my_set_nonblock(m_sockpair[0], false);
    if(!ret) {
        BOOST_LOG_TRIVIAL(error) << "Timer set sockpair[0] block failed";
        return ret;
    }
    ret = my_set_nonblock(m_sockpair[1], false);
    if(!ret) {
        BOOST_LOG_TRIVIAL(error) << "Timer set sockpair[1] block failed";
        return ret;
    }
    return ret;
}

void MyTimerTask::CloseSockPair()
{
    if(-1 == close(m_sockpair[0])){
        BOOST_LOG_TRIVIAL(error) << "Timer close sockpair[0]: " << my_get_error();
    }
    if(-1 == close(m_sockpair[1])){
        BOOST_LOG_TRIVIAL(error) << "Timer close sockpair[1]: " << my_get_error();
    }
}
