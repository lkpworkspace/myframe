#include "MyMsg.h"

void MyMsg::SetData(const char* data, unsigned int len)
{
    _data.clear();
    _data.append(data, len);
}
void MyMsg::SetData(const std::string& data)
{
    SetData(data.data(), data.size());
}