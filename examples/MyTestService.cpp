#include "MyModule.h"
#include "MyFrame.h"
#include "MyContext.h"
#include <iostream>
#include <unistd.h>
#include <string.h>

class MyTestService : public MyModule
{
public:
    MyTestService(){}
    virtual ~MyTestService(){}

    virtual int Init(MyContext* c, const char* param) override
    {
        std::cout << "MyTestService init" << std::endl;
        my_send(c, 0, 0x0, 0, 0, (void*)"init msg", 8);
        my_callback(c, CB, nullptr);
        usleep(1000 * 500);
        return 0;
    }

    static int CB(MyContext* context, void *ud, int type, int session, uint32_t source , const void *msg, size_t sz)
    {
        const char* send_msg = "hello, world";
        std::string str((char*)msg,sz);
        std::cout << m_srv << " from " << source << " to " << my_handle(context) << ": " << str << ", " << m_count++ << std::endl;
        my_send(context, 0, 0x1, 0, 0, (void*)send_msg, strlen(send_msg));
        usleep(1000 * 100);
    }
    static int m_count;
    static std::string m_srv;
};

int MyTestService::m_count = 0;
std::string MyTestService::m_srv = "MyTestService: ";

extern "C" MyModule* my_mod_create()
{
    return static_cast<MyModule*>(new MyTestService());
}

extern "C" void my_mod_destory(MyModule* m)
{
    delete m;
}
