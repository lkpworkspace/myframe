#include <string.h>

#include "MySock.h"

MySock::MySock() :
    m_state(0),
    m_handle(0),
    m_proto(PROTOCOL_UNKNOWN),
    m_fd(-1)
{
    memset((void*)&m_addr, 0, sizeof(m_addr));
}

MySock::~MySock()
{}

int MySock::GetEventType()
{
    return EV_SOCK;
}

unsigned int MySock::GetEpollEventType()
{
    return (EPOLLIN | EPOLLONESHOT);
}

MyObj *MySock::CB(MyEvent*)
{
    return static_cast<MyObj*>(&m_send);
}
