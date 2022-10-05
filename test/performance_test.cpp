/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include <list>
#include <memory>
#include <thread>
#include <gtest/gtest.h>

#include "myframe/MyMsg.h"
#include "myframe/MyLog.h"
#include "myframe/MyFlags.h"
#include "myframe/MyApp.h"
#include "myframe/MyActor.h"
#include "myframe/MyModManager.h"

using namespace myframe;

class TransMsgCostTest : public MyActor
{
public:
    TransMsgCostTest()
        : _msg(8192, 'x')
    {}

    int Init(const char* param) override {
        LOG(INFO) << "begin runing TransMsgCostTest";
        _last = std::chrono::high_resolution_clock::now();
        _begin = std::chrono::high_resolution_clock::now();
        Send(GetActorName(), std::make_shared<MyMsg>(_msg));
        return 0;
    }

    void Proc(const std::shared_ptr<const MyMsg>& msg) override {
        auto now = std::chrono::high_resolution_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(now - _last).count();
        LOG(INFO) << GetActorName() << " trans msg cost(us) " << us;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        _last = std::chrono::high_resolution_clock::now();

        auto sec = std::chrono::duration_cast<std::chrono::seconds>(_last - _begin).count();
        if (sec < 60) {
            Send(GetActorName(), std::make_shared<MyMsg>(_msg));
        } else {
            LOG(INFO) << "runing next test...";
            Send("actor.Trans10ActorCostTest.0", std::make_shared<MyMsg>(_msg));
        }
    }

private:
    std::chrono::high_resolution_clock::time_point _begin;
    std::chrono::high_resolution_clock::time_point _last;
    std::string _msg;
};

class Trans10ActorCostTest : public MyActor {
public:
    Trans10ActorCostTest()
        : _msg(8192, 'y')
    {}

    int Init(const char* param) override {
        _task_num = std::stoi(param);
        LOG(INFO) << "init trans msg num " << _task_num;
        return 0;
    }

    void Proc(const std::shared_ptr<const MyMsg>& msg) override {
        if (!_init) {
            _init = true;
            _total = std::chrono::high_resolution_clock::now();
        }
        if (_task_num == 0) {
            _begin = std::chrono::high_resolution_clock::now();
        }
        if (_task_num == 9) {
            auto now = std::chrono::high_resolution_clock::now();
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(now - _begin).count();
            LOG(INFO) << GetActorName() << " trans 10 actor msg cost(us) " << us;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            _begin = std::chrono::high_resolution_clock::now();
        }
        auto sec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - _total).count();
        std::string next_actor_name = "actor.Trans10ActorCostTest." + std::to_string((_task_num + 1) % 10);
        if (sec < 60) {
            Send(next_actor_name, std::make_shared<MyMsg>(_msg));
        } else {
            LOG(INFO) << "runing next test...";
            Send("actor.FullSpeedTransTest.0", std::make_shared<MyMsg>(_msg));
        }
    }

private:
    static bool _init;
    static std::chrono::high_resolution_clock::time_point _total;
    static std::chrono::high_resolution_clock::time_point _begin;
    int _task_num{0};
    std::string _msg;
};
bool Trans10ActorCostTest::_init{false};
std::chrono::high_resolution_clock::time_point Trans10ActorCostTest::_total;
std::chrono::high_resolution_clock::time_point Trans10ActorCostTest::_begin;

class FullSpeedTransTest : public MyActor {
public:
    FullSpeedTransTest()
        : _msg(8192, 'z')
    {}

    int Init(const char* param) override {
        LOG(INFO) << "init full speed trans ";
        return 0;
    }

    void Proc(const std::shared_ptr<const MyMsg>& msg) override {
        if (!_init) {
            _init = true;
            _begin = std::chrono::high_resolution_clock::now();
            _last = std::chrono::high_resolution_clock::now();
        }
        auto now = std::chrono::high_resolution_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(now - _last).count();
        _cnt++;
        if (us / 1000.0 > 1000.0) {
            LOG(INFO) << GetActorName() << ": full speed msg count " << _cnt;
            _cnt = 0;
            _last = std::chrono::high_resolution_clock::now();
        }
        auto sec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - _begin).count();
        if (sec < 60) {
            Send(GetActorName(), std::make_shared<myframe::MyMsg>());
        } else {
            LOG(INFO) << "runing next test...";
            for (int i = 0; i < 20; ++i) {
                std::string name = "actor.FullSpeed20ActorTransTest." + std::to_string(i);
                Send(name, std::make_shared<MyMsg>(_msg));
            }
        }
    }

private:
    bool _init{false};
    int _cnt{0};
    std::chrono::high_resolution_clock::time_point _begin;
    std::chrono::high_resolution_clock::time_point _last;
    std::string _msg;
};

class FullSpeed20ActorTransTest : public MyActor {
public:
    FullSpeed20ActorTransTest()
        : _msg(8192, 'j')
    {}

    int Init(const char* param) override {
        LOG(INFO) << "init full speed 20 actor trans ";
        return 0;
    }

    void Proc(const std::shared_ptr<const MyMsg>& msg) override {
        if (!_init) {
            _init = true;
            _begin = std::chrono::high_resolution_clock::now();
            _last = std::chrono::high_resolution_clock::now();
        }
        auto now = std::chrono::high_resolution_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(now - _last).count();
        _cnt++;
        if (us / 1000.0 > 1000.0) {
            LOG(INFO) << GetActorName() << ": full speed 20 actor msg count " << _cnt;
            _cnt = 0;
            _last = std::chrono::high_resolution_clock::now();
        }
        auto sec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - _begin).count();
        if (sec < 60) {
            Send(GetActorName(), std::make_shared<myframe::MyMsg>());
        } else {
            if (!_is_send) {
                _is_send = true;
                LOG(INFO) << "runing next test...";
                for (int i = 0; i < 100; ++i) {
                    std::string name = "actor.FullSpeed100ActorTransTest." + std::to_string(i);
                    Send(name, std::make_shared<MyMsg>(_msg));
                }
            }
        }
    }

private:
    static std::atomic_bool _is_send;
    bool _init{false};
    int _cnt{0};
    std::chrono::high_resolution_clock::time_point _begin;
    std::chrono::high_resolution_clock::time_point _last;
    std::string _msg;
};
std::atomic_bool FullSpeed20ActorTransTest::_is_send{false};

class FullSpeed100ActorTransTest : public MyActor {
public:
    FullSpeed100ActorTransTest()
        : _msg(8192, 'k')
    {}

    int Init(const char* param) override {
        LOG(INFO) << "init full speed 100 actor trans ";
        return 0;
    }

    void Proc(const std::shared_ptr<const MyMsg>& msg) override {
        if (!_init) {
            _init = true;
            _begin = std::chrono::high_resolution_clock::now();
            _last = std::chrono::high_resolution_clock::now();
        }
        auto now = std::chrono::high_resolution_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(now - _last).count();
        _cnt++;
        if (us / 1000.0 > 1000.0) {
            LOG(INFO) << GetActorName() << ": full speed 100 actor msg count " << _cnt;
            _cnt = 0;
            _last = std::chrono::high_resolution_clock::now();
        }
        auto sec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - _begin).count();
        if (sec < 60) {
            Send(GetActorName(), std::make_shared<myframe::MyMsg>());
        }
    }

private:
    bool _init{false};
    int _cnt{0};
    std::chrono::high_resolution_clock::time_point _begin;
    std::chrono::high_resolution_clock::time_point _last;
    std::string _msg;
};

TEST(MyApp, performance_test) {
    myframe::MyLog log;

    myframe::FLAGS_worker_count = 4;

    auto app = std::make_shared<myframe::MyApp>();
    if(false == app->Init()) {
        LOG(ERROR) << "Init failed";
        return;
    }

    // mod manager
    auto& mod = app->GetModManager();

    // 发送单条消息耗时（测试时长1分钟，每隔10毫秒发送1条消息）
    // 耗时
    //   平均值:
    //   99分位:
    {
        mod->RegActor("TransMsgCostTest", [](const std::string&){ return std::make_shared<TransMsgCostTest>(); });
        auto actor = mod->CreateActorInst("class", "TransMsgCostTest");
        app->AddActor("#1", "", actor);
    }

    // 消息流转10个actor耗时（测试时长1分钟，每隔10毫秒发送1条消息）
    // 耗时
    //   平均值:
    //   99分位:
    {
        mod->RegActor("Trans10ActorCostTest", [](const std::string&){ return std::make_shared<Trans10ActorCostTest>(); });
        for (int i = 0; i < 10; ++i) {
            auto actor = mod->CreateActorInst("class", "Trans10ActorCostTest");
            app->AddActor(std::to_string(i), std::to_string(i), actor);
        }
    }

    // 1个actor消息吞吐量（测试时长1分钟，全速运行）
    // 消息个数:
    // 消息大小:
    {
        mod->RegActor("FullSpeedTransTest", [](const std::string&){ return std::make_shared<FullSpeedTransTest>(); });
        auto actor = mod->CreateActorInst("class", "FullSpeedTransTest");
        app->AddActor("0", std::to_string(0), actor);
    }

    // 20个actor消息吞吐量（测试时长1分钟，全速运行）
    // 消息个数:
    // 消息大小:
    {
        mod->RegActor("FullSpeed20ActorTransTest", [](const std::string&){ return std::make_shared<FullSpeed20ActorTransTest>(); });
        for (int i = 0; i < 20; ++i) {
            auto actor = mod->CreateActorInst("class", "FullSpeed20ActorTransTest");
            app->AddActor(std::to_string(i), std::to_string(i), actor);
        }
    }

    // 100个actor消息吞吐量（测试时长1分钟，全速运行）
    // 消息个数:
    // 消息大小:
    {
        mod->RegActor("FullSpeed100ActorTransTest", [](const std::string&){ return std::make_shared<FullSpeed100ActorTransTest>(); });
        for (int i = 0; i < 100; ++i) {
            auto actor = mod->CreateActorInst("class", "FullSpeed100ActorTransTest");
            app->AddActor(std::to_string(i), std::to_string(i), actor);
        }
    }

    app->Exec();
}