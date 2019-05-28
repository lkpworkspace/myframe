#ifndef __MYEVENT_H__
#define __MYEVENT_H__

#include <sys/epoll.h>

#include "MyCommon.h"
#include "MyList.h"

class MyEvent : public MyNode
{
public:
    enum ENUM_EVENT_TYPE{
        EV_THREAD,
        EV_WORKER,
        EV_SOCK,
        EV_FILE,
        EV_NONE
    };

public:
    MyEvent();
    virtual ~MyEvent();

    /* 事件类型 */
    virtual int GetEventType() = 0;

    /* 获得当前事件的文件描述符 */
    virtual int GetFd(){ return -1; }

    /**
     * 监听的是文件描述符的写事件还是读事件
     * 一般是读或写事件(EPOLLIN/EPOLLOUT)
     */
    virtual unsigned int GetEpollEventType() = 0;

    /* 节点类型 */
    virtual enum ENUM_NODE_TYPE GetNodeType() { return NODE_EVENT; }

    /* 事件的回调函数 */
    virtual MyObj* CB(MyEvent*) = 0;
};

#endif // __MYEVENT_H__
