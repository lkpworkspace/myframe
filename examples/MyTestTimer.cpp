#include <iostream>
#include <string.h>

#include "myframe/MyModule.h"
#include "myframe/MyMsg.h"

class MyTestTimer : public MyModule
{
public:
    MyTestTimer(){}
    virtual ~MyTestTimer(){}

    int Init(const char* param) override {
        /* 设置超时时间为 10 * 10 ms */
        Timeout(10, 0xff);
        return 0;
    }

    void CB(std::shared_ptr<MyMsg>& msg) override {
        switch(msg->GetMsgType()){
            case MyMsg::MyMsgType::RESPONSE: {
                auto rmsg = std::dynamic_pointer_cast<MyRespMsg>(msg);
                if(rmsg->GetRespMsgType() == MyRespMsg::MyRespMsgType::TIMER){
                    /* 设置下一次超时时间 100 * 10 ms */
                    Timeout(100, 0xff);

                    std::cout << "----> from " << msg->source << " to " 
                        << GetServiceName() << ": " << "timeout" << std::endl;
                }
                break;
            }
            default:
                /* 忽略其它消息 */
                std::cout << "Unknown msg type" << std::endl;
                break;
        }
    }
};

extern "C" std::shared_ptr<MyModule> my_mod_create(const std::string& service_name) {
    return std::make_shared<MyTestTimer>();
}
