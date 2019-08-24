#include "MyMsg.h"

void MyTextMsg::SetData(const char* data, int len)
{
    m_data.clear();
    m_data.append(data, len);
}
void MyTextMsg::SetData(std::string& data)
{
    m_data.clear();
    m_data.append(data.data(), data.size());
}


MySockMsg::MySockMsg() :
    m_id(-1),
    m_type(MySockMsgType::NONE)
{
    m_data.clear();
}

void MySockMsg::SetData(const char* data, int len)
{
    m_data.clear();
    m_data.append(data, len);
}

void MySockMsg::SetData(std::string& data)
{
    m_data.clear();
    m_data.append(data.data(), data.size());
}