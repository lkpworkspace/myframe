#ifndef __MySock_H__
#define __MySock_H__
#include <sys/socket.h>
#include <netinet/in.h>
#include "MyCommon.h"
#include "MyEvent.h"

class MyMsg;
class MySocksMgr;
class MySock : public MyEvent
{
    friend class MySocksMgr;
public:
    enum ENUM_PROTOCOL_TYPE{
        PROTOCOL_TCP,
        PROTOCOL_UDP,
        PROTOCOL_UDPv6,
        PROTOCOL_UNKNOWN
    };
    enum ENUM_SOCKET_STATE_TYPE{
        SOCK_STATE_INVALID,
        SOCK_STATE_RESERVE,
        SOCK_STATE_LISTEN,
        SOCK_STATE_CONNECTING,
        SOCK_STATE_CONNECTED,
        SOCK_STATE_HALFCLOSE,
        SOCK_STATE_ACCEPT,
        SOCK_STATE_BIND,
    };
public:
    MySock();
    virtual ~MySock();

    /**
     * 重写 MyEvent 的虚函数
     */
    virtual int GetEventType() override;
    virtual unsigned int GetEpollEventType() override;
    virtual MyList* CB(MyEvent*ev, int *ud) override;
    virtual int GetFd(){ return m_fd; }
    virtual void SetEpollEvents(uint32_t ev) override
    { m_epoll_events = ev; }

    void SetEpollWrite(bool b);

    void SetState(int state) { m_state = state; }
    int GetState() { return m_state; }
    int GetType() { return m_type; }

    int Send(const void* buffer, int sz);
private:
    void SendWriteList();
    MyMsg* CloseLater();
    int                       m_rd_size;
    int64_t                   m_wb_size; // 目前没用
    int64_t                   m_warn_size; // 目前没用
    MyList                    m_write_list;// 写缓存链表, 目前没用
    MyList                    m_send; // 目前没啥用
    int                       m_state;// enum ENUM_SOCKET_STATE_TYPE
    int                       m_type;// enum ENUM_PROTOCOL_TYPE
    uint32_t                  m_epoll_events; // 等待到的事件类型
    unsigned int              m_epoll_flag; // 设置要监听的事件类型
    int                       m_fd;
    uint32_t                  m_id;
    uint32_t                  m_handle;
};

#endif
