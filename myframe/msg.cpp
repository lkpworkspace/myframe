/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.

Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "myframe/msg.h"

#include <string.h>

namespace myframe {

Msg::Msg(const char* data) {
  SetData(data, strlen(data));
}

Msg::Msg(const char* data, int len) {
  SetData(data, len);
}

Msg::Msg(const std::string& data) {
  SetData(data);
}

void Msg::SetData(const char* data, unsigned int len) {
  data_.clear();
  data_.append(data, len);
}
void Msg::SetData(const std::string& data) {
  SetData(data.data(), data.size());
}
void Msg::SetAnyData(const std::any& any_data) {
  type_ = any_data.type().name();
  any_data_ = any_data;
}

std::ostream& operator<<(std::ostream& out, const Msg& msg) {
  out << "[" << msg.GetSrc() << " to "
    << msg.GetDst() << "](" << msg.GetType() << ")";
  return out;
}

}  // namespace myframe

