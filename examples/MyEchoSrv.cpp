#include <iostream>
#include <string.h>
#include <stdio.h>

#include "MyModule.h"
#include "MyFrame.h"
#include "MyContext.h"


class MyEchoSrv : public MyModule
{
public:
    MyEchoSrv(){}
    virtual ~MyEchoSrv(){}

    virtual int Init(MyContext* c, const char* param) override
    {
		uint32_t handle = my_handle(c);
		my_callback(c, CB, nullptr);
        m_tcp_srv_id = my_listen(c, "127.0.0.1", 9510, 0);
        if(m_tcp_srv_id == -1)
            printf("Listen on port 9510 failed\n");
        return 0;
    }

    static int CB(MyContext* context, void *ud, int type, int session, uint32_t source , const void *msg, size_t sz)
    {
        struct my_sock_msg* sock_msg;
        switch(type){
        case MY_PTYPE_SOCKET:
            sock_msg = (struct my_sock_msg*)msg;
            switch(sock_msg->type){
            case MY_SOCKET_TYPE_DATA:
                my_sock_send(sock_msg->id, sock_msg->buffer, sock_msg->ud);
                break;
            case MY_SOCKET_TYPE_ACCEPT:
                printf("Client %d connected\n", sock_msg->id);
                fflush(stdout);
                break;
            case MY_SOCKET_TYPE_CLOSE:
                printf("Client %d disconnect\n", sock_msg->id);
                fflush(stdout);
                break;
            }
            if(sock_msg->buffer)
                free(sock_msg->buffer);
            break;
        }
        return 0;
    }

    static int m_tcp_srv_id;
};

int MyEchoSrv::m_tcp_srv_id = -1;

extern "C" MyModule* my_mod_create()
{
    return static_cast<MyModule*>(new MyEchoSrv());
}

extern "C" void my_mod_destory(MyModule* m)
{
    delete m;
}