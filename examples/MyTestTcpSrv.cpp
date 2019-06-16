#include "MyModule.h"
#include "MyFrame.h"
#include "MyContext.h"
#include <iostream>
#include <unistd.h>
#include <string.h>

void PrintSockMsg(struct my_sock_msg* msg)
{
    printf("type: %d\n",msg->type);
    printf("id: %d\n",msg->id);
    printf("size: %d\n",msg->ud);
    printf("data: %.*s\n",msg->ud, msg->buffer);
    fflush(stdout);
}

class MyTestTcpSrv : public MyModule
{
public:
    MyTestTcpSrv() :
    m_count(0),
    m_srv("MyTestTcpSrv: ")
    {}
    virtual ~MyTestTcpSrv(){}

    virtual int Init(MyContext* c, const char* param) override
    {
        std::cout << "MyTestTcpSrv init" << std::endl;
        my_callback(c, CB, this);
        my_listen(c, "127.0.0.1", 9510, 0);
        return 0;
    }

    static int CB(MyContext* context, void *ud, int type, int session, uint32_t source , const void *msg, size_t sz)
    {
        MyTestTcpSrv* self = static_cast<MyTestTcpSrv*>(ud);

        std::cout << self->m_srv << " from " << source << " to " << my_handle(context) << ": size " << sz << ", " << self->m_count++ << std::endl;
        struct my_sock_msg* sock_msg = (struct my_sock_msg*)msg;
        PrintSockMsg(sock_msg);
        free(sock_msg->buffer);
        return 0;
    }

    int m_count;
    std::string m_srv;
};

extern "C" MyModule* my_mod_create()
{
    return static_cast<MyModule*>(new MyTestTcpSrv());
}

extern "C" void my_mod_destory(MyModule* m)
{
    delete m;
}
