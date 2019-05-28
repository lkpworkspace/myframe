#ifndef __MySock_H__
#define __MySock_H__
#include <sys/socket.h>
#include <netinet/in.h>
#include "MyCommon.h"
#include "MyEvent.h"

class MySock : public MyEvent
{
public:
    enum ENUM_PROTOCOL_TYPE{
        PROTOCOL_TCP,
        PROTOCOL_UDP,
        PROTOCOL_UDPv6,
        PROTOCOL_UNKNOWN
    };
    union sockaddr_all{
        struct sockaddr s;
        struct sockaddr_in v4;
        struct sockaddr_in6 v6;
    };
public:
    MySock();
    virtual ~MySock();

    /**
     * 重写 MyEvent 的虚函数
     */
    virtual int GetEventType() override;
    virtual unsigned int GetEpollEventType() override;
    virtual MyObj* CB(MyEvent*) override;
    virtual int GetFd(){ return m_fd; }

private:
    /* socket读写数据以消息的方式发送给服务 */
    MyList                    m_send;
    int                       m_state;
    uint32_t                  m_handle;
    enum ENUM_PROTOCOL_TYPE   m_proto;
    int                       m_fd;
    union sockaddr_all        m_addr;
};

#endif
