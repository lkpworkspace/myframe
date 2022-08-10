#pragma once

enum class MyEventType : int {
    EV_WORKER,
    EV_TIMER,
    EV_USER,
};

class MyEvent
{
public:
    MyEvent() {}
    virtual ~MyEvent() {}

    /* 事件类型 */
    virtual MyEventType GetMyEventType() { 
        return MyEventType::EV_USER; 
    }

    /* 获得当前事件的文件描述符 */
    virtual int GetFd() = 0;

    /**
     * 监听的是文件描述符的写事件还是读事件
     * 一般是读或写事件(EPOLLIN/EPOLLOUT)
     */
    virtual unsigned int ListenEpollEventType() = 0;

    /* 获得的epoll事件类型(call by MyApp) */
    virtual void RetEpollEventType(uint32_t ev) = 0;
};
