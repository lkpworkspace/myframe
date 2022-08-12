#pragma once
#include <mutex>
#include <memory>
#include <list>

#include "MyCommon.h"
#include "MyWorker.h"
#include "MyMsg.h"
#include "MyList.h"

#define TVN_BITS 6
#define TVR_BITS 8
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_MASK (TVN_SIZE - 1)
#define TVR_MASK (TVR_SIZE - 1)

#define MY_RESOLUTION_MS 10

class MyTimer : public MyNode
{
    friend class MyTimerManager;
public:
    MyTimer(){}
    virtual ~MyTimer(){}

    std::string _actor_name;
    std::string _timer_name;
    uint32_t m_expire;        // interval
    bool     m_run;
};

class MyTimerManager
{
public:
    MyTimerManager();
    virtual ~MyTimerManager();

    int Timeout(const std::string& actor_name, const std::string& timer_name, int time);

    std::list<std::shared_ptr<MyMsg>>& Updatetime();

private:
    void _AddTimerNode(MyTimer* node);
    void _Updatetime();
    void _Execute();
    void _MoveList(int level, int idx);
    void _Shift();
    void _Dispath(MyList *cur);

    MyList            _tv1[TVR_SIZE];
    MyList            _tv2[TVN_SIZE];
    MyList            _tv3[TVN_SIZE];
    MyList            _tv4[TVN_SIZE];
    MyList            _tv5[TVN_SIZE];
    MyList*           _tv[4];

    uint32_t          _time;
    uint64_t          _cur_point;

    std::list<std::shared_ptr<MyMsg>> _timeout_list;
    std::mutex        _mutex;
};

class MyWorkerTimer : public MyWorker
{
    friend class MyApp;
public:
    MyWorkerTimer();
    virtual ~MyWorkerTimer();

    int SetTimeout(const std::string& actor_name, const std::string& timer_name, int time);

    /**
     * override MyWorker virtual method
     */
    void Run() override;
    void OnInit() override;
    void OnExit() override;

    /**
     * override MyEvent virtual method
     */
    MyEventType GetMyEventType() override
    { return MyEventType::EV_TIMER; }

private:
    int Work();

    MyTimerManager        _timer_mgr;
};
