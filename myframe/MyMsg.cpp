#include "MyMsg.h"

void MyTextMsg::SetData(const char* data, int len)
{
    _data.clear();
    _data.append(data, len);
}
void MyTextMsg::SetData(std::string& data)
{
    _data.clear();
    _data.append(data.data(), data.size());
}