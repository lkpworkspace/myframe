/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "MyMsg.h"

namespace myframe {

void MyMsg::SetData(const char* data, unsigned int len)
{
    _data.clear();
    _data.append(data, len);
}
void MyMsg::SetData(const std::string& data)
{
    SetData(data.data(), data.size());
}

} // namespace myframe