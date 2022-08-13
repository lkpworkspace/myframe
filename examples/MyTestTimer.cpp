#include <iostream>
#include <string.h>

#include <glog/logging.h>

#include "myframe/MyActor.h"
#include "myframe/MyMsg.h"

class MyTestTimer : public MyActor
{
public:
    MyTestTimer(){}
    virtual ~MyTestTimer(){}

    int Init(const char* param) override {
        /* 设置超时时间为 100 * 10 ms */
        Timeout("1000ms", 10);
        return 0;
    }

    void CB(const std::shared_ptr<const MyMsg>& msg) override {
        if (msg->GetMsgType() == "TIMER" && msg->GetMsgDesc() == "1000ms") {
            const auto& rmsg = std::dynamic_pointer_cast<const MyTextMsg>(msg);
            /* 设置下一次超时时间 100 * 10 ms */
            Timeout("1000ms", 100);
            LOG(INFO) << "----> from " << msg->GetSrc() << " to " 
                << GetActorName() << ": " << "timeout";
        }
    }
};

extern "C" std::shared_ptr<MyActor> my_actor_create(const std::string& actor_name) {
    return std::make_shared<MyTestTimer>();
}
