#include <iostream>
#include <string.h>

#include "MyModule.h"
#include "MyFrame.h"
#include "MyContext.h"


class MyTestTimer : public MyModule
{
public:
    MyTestTimer(){}
    virtual ~MyTestTimer(){}

    virtual int Init(MyContext* c, const char* param) override
    {
        m_handle = my_handle(c);
        my_callback(c, CB, this);
        m_timer = my_timeout(m_handle, 10, 0xff);
        if(m_timer == -1)
            std::cout << "Create timer failed" << std::endl;
        return 0;
    }

    static int CB(MyContext* context, void *ud, int type, int session, uint32_t source , const void *msg, size_t sz)
    {
        MyTestTimer* timer = static_cast<MyTestTimer*>(ud);
        timer->m_timer = my_timeout(timer->m_handle, 1, 0xff);
        if(timer->m_timer == -1)
            std::cout << "Create timer failed" << std::endl;
        std::cout << "----> from " << source << " to " << my_handle(context) << ": " << "timeout" << std::endl;
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
