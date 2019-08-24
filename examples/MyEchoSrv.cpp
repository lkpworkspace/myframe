#include <iostream>
#include <string.h>
#include <stdio.h>

#include "MyModule.h"
#include "MyFrame.h"
#include "MyContext.h"
#include "MyMsg.h"

/*
    该服务实现:
        在 127.0.0.1:9510 启动TCP监听
        客户端连接会打印 "Client %d connected"
        客户端发送数据，服务端会把数据再回传给客户端
        客户端断开会打印 "Client %d disconnect"
*/
class MyEchoSrv : public MyModule
{
public:
    MyEchoSrv(){}
    virtual ~MyEchoSrv(){}

    virtual int Init(MyContext* c, const char* param) override
    {
		uint32_t handle = my_handle(c);
		my_callback(c, CB, this);
        m_tcp_srv_id = my_listen(c, "127.0.0.1", 9510, 0);
        if(m_tcp_srv_id == -1)
            printf("Listen on port 9510 failed\n");
        return 0;
    }

    static int CB(MyContext* context, MyMsg* msg, void* ud)
    {
        MyEchoSrv* self = static_cast<MyEchoSrv*>(ud);
        MySockMsg* smsg = nullptr;

        switch(msg->GetMsgType()){
        case MyMsg::MyMsgType::SOCKET:
            smsg = static_cast<MySockMsg*>(msg);
            switch(smsg->GetSockMsgType()){
            case MySockMsg::MySockMsgType::DATA:
                my_sock_send(smsg->GetSockId(), smsg->GetData().data(), smsg->GetData().size());
                break;
            case MySockMsg::MySockMsgType::ACCEPT:
                printf("Client %d connected\n", smsg->GetSockId());
                fflush(stdout);
                break;
            case MySockMsg::MySockMsgType::CLOSE:
                printf("Client %d disconnect\n", smsg->GetSockId());
                fflush(stdout);
                break;
            }
            break;
        }
        return 0;
    }

    uint32_t m_handle;
    int      m_tcp_srv_id;
};

extern "C" MyModule* my_mod_create()
{
    return static_cast<MyModule*>(new MyEchoSrv());
}

extern "C" void my_mod_destory(MyModule* m)
{
    delete m;
}