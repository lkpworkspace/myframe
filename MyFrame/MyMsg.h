#ifndef __MYMSG_H__
#define __MYMSG_H__
#include <cstdint>
#include <string>

#include "MyCommon.h"
#include "MyList.h"

class MyMsg : public MyNode
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
    virtual ~MyMsg(){}

    uint32_t    source;
    uint32_t    destination;
    int         session;
    MyMsgCtrl   ctrl;

    MyMsgCtrl GetCtrl() { return ctrl; }
    void SetCtrl(MyMsgCtrl c) { ctrl = c; }

    virtual MyMsgType GetMsgType() { return MyMsgType::NONE; }

protected:

    /* 节点类型 */
    virtual enum ENUM_NODE_TYPE GetNodeType() override { return NODE_MSG; }
};

class MyTextMsg : public MyMsg
{
public:
    MyTextMsg() :
        m_data("")
    {}
    virtual ~MyTextMsg(){}

    virtual MyMsgType GetMsgType() override { return MyMsgType::TEXT; }

    void SetData(const char* data, int len);
    void SetData(std::string& data);
    std::string& GetData() { return m_data; }
private:
    std::string         m_data;
};

class MyRespMsg : public MyMsg
{
public:
    enum class MyRespMsgType : int {
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

class MySockMsg : public MyMsg
{
public:
    enum class MySockMsgType :int {
        NONE           = -1,    // 未知类型
        DATA           = 1,     // 数据
        CONNECT        = 2,     // 套接字连接
        CLOSE          = 3,     // 套接字关闭
        ACCEPT         = 4,     // 新的客户端连接
        ERROR          = 5,     // 套接字错误
        UDP            = 6,     // udp数据
        WARNING        = 7,     // 警告，例如 缓存超过 1MB 警告
    };
    MySockMsg();
    virtual ~MySockMsg(){}

    virtual MyMsgType GetMsgType() override { return MyMsgType::SOCKET; }

    MySockMsgType GetSockMsgType() { return m_type; }
    void SetSockMsgType(MySockMsgType type) { m_type = type; }

    int GetSockId() { return m_id; }
    void SetSockId(int id) { m_id = id; }

    void SetData(const char* data, int len);
    void SetData(std::string& data);
    std::string& GetData() { return m_data; }
private:
    int                 m_id;
    MySockMsgType       m_type;
    std::string         m_data;
};

#endif // __MYEVENT_H__
