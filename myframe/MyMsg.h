#ifndef __MYMSG_H__
#define __MYMSG_H__
#include <cstdint>
#include <string>

#include "MyCommon.h"

class MyMsg
{
public:

    enum class MyMsgType : int {
        NONE            = -1,    // 未知消息
        TEXT            = 0,     // 文本消息
        RESPONSE        = 1,     // 回复消息
        MULTICAST       = 2,     // 广播
        SOCKET          = 6,     // 套接字消息
    };

    enum class MyMsgCtrl : int {
        ALLOC_SESSION   = 0x20000,
    };

    MyMsg() :
        source(0),
        destination(0),
        session(0)
    {}
    MyMsg(uint32_t dst) :
        source(0),
        destination(dst),
        session(0)
    {}
    virtual ~MyMsg(){}

    /* 服务源句柄号 */
    uint32_t    source;
    /* 服务目的句柄号 */
    uint32_t    destination;
    /* 未使用 */
    int         session;
    MyMsgCtrl   ctrl;

    MyMsgCtrl GetCtrl() { return ctrl; }
    void SetCtrl(MyMsgCtrl c) { ctrl = c; }

    virtual MyMsgType GetMsgType() { return MyMsgType::NONE; }
};

/**
 * 文本消息(也可以用于传输二进制消息)
 */
class MyTextMsg : public MyMsg
{
public:
    MyTextMsg() :
        m_data("")
    {}
    MyTextMsg(uint32_t dst, std::string dat) :
        MyMsg(dst),
        m_data(dat)
    {}
    virtual ~MyTextMsg(){}

    virtual MyMsgType GetMsgType() override { return MyMsgType::TEXT; }

    void SetData(const char* data, int len);
    void SetData(std::string& data);
    std::string& GetData() { return m_data; }
private:
    std::string         m_data;
};

/**
 * 回复消息
 */
class MyRespMsg : public MyMsg
{
public:
    /* 回复消息类型 */
    enum class MyRespMsgType : int {
        /* 定时器超时 */
        TIMER            = 0,
    };
    MyRespMsg() :
        m_data("")
    {}
    virtual ~MyRespMsg(){}

    virtual MyMsgType GetMsgType() override { return MyMsgType::RESPONSE; }

    MyRespMsgType GetRespMsgType() { return m_type; }
    void SetRespMsgType(MyRespMsgType type) { m_type = type; }

    void SetData(const char* data, int len);
    void SetData(std::string& data);
    std::string& GetData() { return m_data; }
private:
    std::string         m_data;
    MyRespMsgType       m_type;
};

#endif // __MYEVENT_H__
