/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <thread>
#include <chrono>

#include <glog/logging.h>

#include "myframe/MyCUtils.h"
#include "myframe/MyCommon.h"
#include "myframe/MyMsg.h"
#include "myframe/MyActor.h"
#include "myframe/MyWorker.h"

using namespace myframe;

template<typename T>
class MyQueue final {
public:
    MyQueue() {
        CreateSockPair();
    }
    ~MyQueue() {
        CloseSockPair();
    }

    int GetFd0() { return _fd_pair[0]; }
    int GetFd1() { return _fd_pair[1]; }

    void Push(std::shared_ptr<T> data) {
        _data = data;
        char cmd_char = 'p';
        write(_fd_pair[0], &cmd_char, 1);
        read(_fd_pair[0], &cmd_char, 1);
    }

    std::shared_ptr<T> Pop() {
        std::shared_ptr<T> ret = nullptr;
        char cmd_char = '\0';
        read(_fd_pair[1], &cmd_char, 1);
        ret = _data;
        _data = nullptr;
        write(_fd_pair[1], &cmd_char, 1);
        return ret;
    }

private:
    bool CreateSockPair(){
        int res = -1;
        bool ret = true;

        res = socketpair(AF_UNIX, SOCK_DGRAM, 0, _fd_pair);
        if(res == -1) {
            LOG(ERROR) << "create sockpair failed";
            return false;
        }
        ret = my_set_nonblock(_fd_pair[0], false);
        if(!ret) {
            LOG(ERROR) << "set sockpair[0] block failed";
            return ret;
        }
        ret = my_set_nonblock(_fd_pair[1], false);
        if(!ret) {
            LOG(ERROR) << "set sockpair[1] block failed";
            return ret;
        }
        return ret;
    }
    void CloseSockPair() {
        if(-1 == close(_fd_pair[0])){
            LOG(ERROR) << "close sockpair[0]: " << my_get_error();
        }
        if(-1 == close(_fd_pair[1])){
            LOG(ERROR) << "close sockpair[1]: " << my_get_error();
        }
    }
    
    std::shared_ptr<T> _data;
    int _fd_pair[2] = {-1, -1};
};

/**
 * @brief 与其它程序或框架交互示例
 */
class ExampleWorkerInteractiveWith3rdFrame : public MyWorker
{
public:
    ExampleWorkerInteractiveWith3rdFrame() {}
    virtual ~ExampleWorkerInteractiveWith3rdFrame() {}

    void OnInit() override {
        // 线程_th通过MyQueue与myframe交互
        _th = std::thread([this](){
            while(1) {
                _queue.Push(std::make_shared<std::string>("this is 3rd frame"));
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
        _th.detach();

        // 通知myframe该worker可以接收来自myframe的消息
        SendCmdToMain(myframe::MyWorkerCmd::WAIT_FOR_MSG);
    }
    
    void Run() override {
        bool has_main_msg = false;
        struct pollfd fds[] = {
            {GetWorkerFd(), POLLIN, 0},
            {_queue.GetFd1(), POLLIN, 0}
        };
        // 等待来自queue或者myframe的消息
        poll(fds, 2, -1);
        if (fds[0].revents & POLLIN) {
            has_main_msg = true;
        }
        if (fds[1].revents & POLLIN) {
            auto data = _queue.Pop();
            // TODO
            // 可以将queue里的消息发给myfrmae的worker或actor
            // eg: Send("actor.xx.xx", std::make_shared<MyMsg>(data->c_str()));
            LOG(INFO) << "get msg from queue: " << data->c_str();
        }
        OnMainMsg(has_main_msg);
    }

    // 分发消息、处理来自myframe的消息
    void OnMainMsg(bool has_main_msg) {
        if (!has_main_msg) {
            SendCmdToMain(myframe::MyWorkerCmd::IDLE);
        }
        while (1) {
            myframe::MyWorkerCmd cmd;
            RecvCmdFromMain(cmd);
            if(myframe::MyWorkerCmd::RUN == cmd) {
                return;
            }
            if (myframe::MyWorkerCmd::RUN_WITH_MSG == cmd) {
                ProcessMainMsg();
                SendCmdToMain(myframe::MyWorkerCmd::WAIT_FOR_MSG);
                if (has_main_msg) {
                    return;
                }
            }
        }
    }

    void ProcessMainMsg() {
        while (RecvMsgListSize() > 0) {
            const auto& msg = GetRecvMsg();
            // TODO 
            LOG(INFO) << "get msg from main " << msg->GetData();
        }
    }

private:
    std::thread _th;
    MyQueue<std::string> _queue;
};

class ExampleActorInteractiveWith3rdFrame : public MyActor
{
public:
    int Init(const char* param) override {
        Timeout("100ms", 10);
        return 0;
    }

    void Proc(const std::shared_ptr<const MyMsg>& msg) override {
        if (msg->GetMsgType() == "TIMER") {
            Send("worker.example_worker_interactive_with_3rd_frame.#1", std::make_shared<MyMsg>("this is interactive_with_3rd_frame actor"));
            Timeout("100ms", 10);
        }
    }
};

/* 创建worker实例函数 */
extern "C" std::shared_ptr<MyWorker> my_worker_create(const std::string& worker_name) {
    return std::make_shared<ExampleWorkerInteractiveWith3rdFrame>();
}

/* 创建actor实例函数 */
extern "C" std::shared_ptr<MyActor> my_actor_create(const std::string& actor_name) {
    return std::make_shared<ExampleActorInteractiveWith3rdFrame>();
}