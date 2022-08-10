#pragma once
#include <cstdint>
#include <string>

#include "MyCommon.h"

class MyMsg
{
public:
    virtual ~MyMsg() {}
    const std::string& GetMsgType() const { return _type; }
    void SetMsgType(const std::string& type) { _type = type; }
    void SetSrc(const std::string& src) { _src = src; }
    const std::string& GetSrc() const { return _src; }
    void SetDst(const std::string& dst) { _dst = dst; }
    const std::string& GetDst() const { return _dst; }

private:
    std::string _src;
    std::string _dst; 
    std::string _type;
};

/**
 * 文本消息(也可以用于传输二进制消息)
 */
class MyTextMsg : public MyMsg
{
public:
    MyTextMsg() :
        _data("") {
        SetMsgType("TEXT");
    }
    MyTextMsg(const std::string& dat) :
        _data(dat) {
        SetMsgType("TEXT");
    }
    virtual ~MyTextMsg(){}
    void SetData(const char* data, int len);
    void SetData(std::string& data);
    const std::string& GetData() { return _data; }
private:
    std::string         _data;
};
