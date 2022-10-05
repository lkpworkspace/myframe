/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include <thread>
#include <chrono>

#include <glog/logging.h>

#include "myframe/MyMsg.h"
#include "myframe/MyActor.h"
#include "myframe/MyWorker.h"

using namespace myframe;

class ExampleWorkerPublic : public MyWorker
{
public:
    ExampleWorkerPublic() {}
    virtual ~ExampleWorkerPublic() {}

    void Run() override {
        DispatchAndWaitMsg();
        while (RecvMsgListSize() > 0) {
            const auto& msg = GetRecvMsg();    
            // TODO: send msg by udp/tcp/zmq/...
            LOG(INFO) << "public msg " << msg->GetData() << " ...";
        }
    }
};

/* 创建worker实例函数 */
extern "C" std::shared_ptr<MyWorker> my_worker_create(const std::string& worker_name) {
    return std::make_shared<ExampleWorkerPublic>();
}
