#include <sys/types.h>
#include <sys/socket.h>

#include "MyWorkerTimer.h"
#include "MyLog.h"
#include "MyCUtils.h"
#include "MyApp.h"
#include "MyMsg.h"
#include "MyActor.h"

MyTimerManager::MyTimerManager()
{
    _tv[0] = _tv2;
    _tv[1] = _tv3;
    _tv[2] = _tv4;
    _tv[3] = _tv5;

    _cur_point = my_gettime_ms() / 10;
}

MyTimerManager::~MyTimerManager()
{
}

void MyTimerManager::_AddTimerNode(MyTimer* node)
{
    uint32_t time = node->m_expire;
    uint32_t cur_time = _time;

    if((time | TVR_MASK) == (cur_time | TVR_MASK)){
        _tv1[time&TVR_MASK].AddTail(node);
    }else{
        int i;
        uint32_t mask = TVR_SIZE << TVN_BITS;
        for(i = 0; i < 3; ++i){
            if ((time|(mask-1))==(cur_time|(mask-1))) {
                break;
            }
            mask <<= TVN_BITS;
        }
        _tv[i][((time >>(TVR_BITS + i*TVN_BITS)) & TVN_MASK)].AddTail(node);
    }
}

int MyTimerManager::Timeout(const std::string& actor_name, const std::string& timer_name, int time)
{
    if(time <= 0) return -1;
    MyTimer* timer = new MyTimer();
    timer->_actor_name = actor_name;
    timer->_timer_name = timer_name;

    // add node
    _mutex.lock();
    timer->m_expire = time + _time;
    _AddTimerNode(timer);
    _mutex.unlock();
    return 0;
}

void MyTimerManager::_Dispath(MyList* cur)
{
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
        timer = dynamic_cast<MyTimer*>(begin);
        auto msg = std::make_shared<MyTextMsg>();
        msg->SetSrc(MY_FRAME_DST_NAME);
        msg->SetDst(timer->_actor_name);
        msg->SetMsgDesc(timer->_timer_name);
        msg->SetMsgType("TIMER");
        delete begin;
        _timeout_list.emplace_back(msg);
        begin = temp;
    }
}

void MyTimerManager::_Execute()
{
    int idx = _time & TVR_MASK;
    while(!_tv1[idx].IsEmpty()){
        _Dispath(&_tv1[idx]);
    }
}
void MyTimerManager::_MoveList(int level, int idx)
{
    MyList* cur = &_tv[level][idx];
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
        timer = dynamic_cast<MyTimer*>(begin);
        _AddTimerNode(timer);
        begin = temp;
    }
}
void MyTimerManager::_Shift()
{
    int mask = TVR_SIZE;
    uint32_t ct = ++_time;
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
void MyTimerManager::_Updatetime()
{
    _mutex.lock();
    _Execute();
    _Shift();
    _Execute();
    _mutex.unlock();
}
std::list<std::shared_ptr<MyMsg>>& MyTimerManager::Updatetime()
{
    uint64_t cp = my_gettime_ms() / MY_RESOLUTION_MS;
    if(cp < _cur_point){
        LOG(ERROR) << "Future time: " << cp << ":" << _cur_point;
        _cur_point = cp;
    }else if(cp != _cur_point){
        uint32_t diff = (uint32_t)(cp - _cur_point);
        _cur_point = cp;
        int i;
        for(i =0; i < diff; ++i){
            _Updatetime();
        }
    }
    return _timeout_list;
}

//////////////////////////////////////////////////////

MyWorkerTimer::MyWorkerTimer() {
    SetInstName("MyWorkerTimer");
}

MyWorkerTimer::~MyWorkerTimer()
{}

void MyWorkerTimer::Run() {
    int dispatch = Work();
    if(dispatch) {
        DispatchMsg();
    }
    usleep(2500);
}

void MyWorkerTimer::OnInit() {
    MyWorker::OnInit();
    LOG(INFO) << "Timer task " << GetPosixThreadId() << " init";
}

void MyWorkerTimer::OnExit() {
    LOG(INFO) << "Timer task " << GetPosixThreadId() << " exit";
    MyWorker::OnExit();
}

int MyWorkerTimer::SetTimeout(const std::string& actor_name, const std::string& timer_name, int time) {
    return _timer_mgr.Timeout(actor_name, timer_name, time);
}

int MyWorkerTimer::Work() {
    auto& timeout_list = _timer_mgr.Updatetime();
    MyListAppend(GetMsgList(), timeout_list);
    return (GetMsgList().empty() == false) ? 1 : 0;
}
