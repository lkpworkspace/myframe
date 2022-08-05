#include <iostream>
#include <string.h>

#include "myframe/MyModule.h"
#include "myframe/MyMsg.h"

class MyTestTimer : public MyModule
{
public:
    MyTestTimer(){}
    virtual ~MyTestTimer(){}

    virtual int Init(const char* param) override
    {
        /* 设置超时时间为 10 * 10 ms */
        Timeout(10, 0xff);
        return 0;
    }

    virtual int CB(MyMsg* msg) override
    {
        MyRespMsg* rmsg = nullptr;
        switch(msg->GetMsgType()){
            case MyMsg::MyMsgType::RESPONSE:
                rmsg = static_cast<MyRespMsg*>(msg);
                if(rmsg->GetRespMsgType() == MyRespMsg::MyRespMsgType::TIMER){
                    /* 设置下一次超时时间 100 * 10 ms */
                    Timeout(100, 0xff);

                    std::cout << "----> from " << GetServiceName(msg->source) << " to " 
                        << GetServiceName() << ": " << "timeout" << std::endl;
                }
                break;
            default:
                /* 忽略其它消息 */
                std::cout << "Unknown msg type" << std::endl;
                break;
        }
        return 1;
    }
};

extern "C" std::shared_ptr<MyModule> my_mod_create() {
    return std::make_shared<MyTestTimer>();
}
