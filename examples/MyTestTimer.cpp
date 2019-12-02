#include <iostream>
#include <string.h>

#include "MyModule.h"
#include "MyFrame.h"
#include "MyContext.h"
#include "MyMsg.h"

class MyTestTimer : public MyModule
{
public:
    MyTestTimer(){}
    virtual ~MyTestTimer(){}

    virtual int Init(MyContext* c, const char* param) override
    {
        m_handle = my_handle(c);
        my_callback(c, CB, this);
        /* 设置超时时间为 10 * 10 ms */
        m_timer = my_timeout(m_handle, 10, 0xff);
        if(m_timer == -1)
            std::cout << "Create timer failed" << std::endl;
        return 0;
    }

    static int CB(MyContext* context, MyMsg* msg, void* ud)
    {
        MyTestTimer* timer = static_cast<MyTestTimer*>(ud);
        
        MyRespMsg* rmsg = nullptr;

        switch(msg->GetMsgType()){
            case MyMsg::MyMsgType::RESPONSE:
                rmsg = static_cast<MyRespMsg*>(msg);
                if(rmsg->GetRespMsgType() == MyRespMsg::MyRespMsgType::TIMER){
                    /* 设置下一次超时时间 100 * 10 ms */
                    timer->m_timer = my_timeout(timer->m_handle, 100, 0xff);
                    if(timer->m_timer == -1)
                        std::cout << "Create timer failed" << std::endl;
                
                    std::cout << "----> from " << msg->source << " to " 
                        << my_handle(context) << ": " << "timeout" << std::endl;
                }
                break;
            default:
                /* 忽略其它消息 */
                std::cout << "Unknown msg type" << std::endl;
                break;
        }
        return 0;
    }

    int m_handle;
    int m_timer;
};

extern "C" MyModule* my_mod_create()
{
    return static_cast<MyModule*>(new MyTestTimer());
}

extern "C" void my_mod_destory(MyModule* m)
{
    delete m;
}
