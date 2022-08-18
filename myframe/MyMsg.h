#pragma once
#include <cstdint>
#include <string>

class MyMsg final
{
public:
    MyMsg()
        : _type("TEXT")
    {}
    MyMsg(const std::string& data)
        : _type("TEXT")
        , _data(data)
    {}

    /**
     * @brief 获得消息源地址
     * @note 来源：actor/worker/timer
     * @return const std::string& 源地址
     */
    const std::string& GetSrc() const { return _src; }
    const std::string& GetDst() const { return _dst; }

    /**
     * @brief 消息类型
     * @note 目前使用到的 "TEXT", "TIMER"; 也可以自定义，用于区分传递给同一个actor的不同消息类型
     * @return const std::string& 消息类型
     */
    const std::string& GetMsgType() const { return _type; }

    /**
     * @brief 消息描述
     * @note 目前myframe timer使用到该函数，见 MyActor::Timeout()
     * 
     * @return const std::string& 消息描述
     */
    const std::string& GetMsgDesc() const { return _desc; }

    /**
     * @brief 数据
     * 
     * @return const std::string& 数据
     */
    const std::string& GetData() const { return _data; }

    void SetSrc(const std::string& src) { _src = src; }
    void SetDst(const std::string& dst) { _dst = dst; }
    void SetMsgType(const std::string& type) { _type = type; }
    void SetMsgDesc(const std::string& desc) { _desc = desc; }
    void SetData(const char* data, unsigned int len);
    void SetData(const std::string& data);

private:
    std::string _src;
    std::string _dst; 
    std::string _type;
    std::string _desc;
    std::string _data;
};
