#include <iostream>
#include <string.h>
#include <stdio.h>

#include "MyModule.h"
#include "MyMsg.h"

/*
    该服务实现:
        在 127.0.0.1:6666 启动TCP监听
        客户端连接会打印 "Client %d connected"
        客户端发送数据，服务端会把数据再回传给客户端
        客户端断开会打印 "Client %d disconnect"
*/
class MyEchoSrv : public MyModule
{
public:
    MyEchoSrv(){}
    virtual ~MyEchoSrv(){}

    virtual int Init(const char* param) override
    {
        m_tcp_srv_id = Listen("127.0.0.1", 6666, 0);
        if(m_tcp_srv_id == -1)
            printf("Listen on port 6666 failed\n");
        return 0;
    }

    virtual int CB(MyMsg* msg) override
    {
        MySockMsg* smsg = nullptr;

        switch(msg->GetMsgType()){
        case MyMsg::MyMsgType::SOCKET:
            smsg = static_cast<MySockMsg*>(msg);
            switch(smsg->GetSockMsgType()){
            case MySockMsg::MySockMsgType::DATA:
                SockSend(smsg->GetSockId(), smsg->GetData().data(), smsg->GetData().size());
                break;
            case MySockMsg::MySockMsgType::ACCEPT:
                printf("Client %d connected\n", smsg->GetSockId());
                break;
            case MySockMsg::MySockMsgType::CLOSE:
                printf("Client %d disconnect\n", smsg->GetSockId());
                break;
            }
            break;
        }
        return 1;
    }

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