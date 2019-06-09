#include "MyModule.h"
#include "MyFrame.h"
#include "MyContext.h"
#include <iostream>
#include <unistd.h>
#include <string.h>

class MyTestService3 : public MyModule
{
public:
    MyTestService3(){}
    virtual ~MyTestService3(){}

    virtual int Init(MyContext* c, const char* param) override
    {
        std::cout << "MyTestService3 init" << std::endl;
        my_send(c, 0, MY_FRAME_DST, 0, 0, (void*)"init msg", 8);
        my_callback(c, CB, nullptr);
        usleep(1000 * 200);
        return 0;
    }

    static int CB(MyContext* context, void *ud, int type, int session, uint32_t source , const void *msg, size_t sz)
    {
        std::string str((char*)msg,sz);
        std::cout << m_srv << " from " << source << " to " << my_handle(context) << ": " << str << ", " << m_count++ << std::endl;
        usleep(1000 * 100);
    }
    static int m_count;
    static std::string m_srv;
};

int MyTestService3::m_count = 0;
std::string MyTestService3::m_srv = "MyTestService3: ";

extern "C" MyModule* my_mod_create()
{
    return static_cast<MyModule*>(new MyTestService3());
}

extern "C" void my_mod_destory(MyModule* m)
{
    delete m;
}
