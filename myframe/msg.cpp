/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/

#include "myframe/msg.h"

#include <string.h>
#include <utility>

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

Msg::Msg(Msg&& o) {
  operator=(std::move(o));
}

Msg::Msg(const Msg& o) {
  operator=(o);
}

void Msg::SetData(const char* data, unsigned int len) {
  data_.clear();
  data_.append(data, len);
}
void Msg::SetData(const std::string& data) {
  SetData(data.data(), data.size());
}
void Msg::SetAnyData(const std::any& any_data) {
  any_data_ = any_data;
}

Msg& Msg::operator=(Msg&& o) noexcept {
  trans_mode_ = o.trans_mode_;
  src_ = std::move(o.src_);
  dst_ = std::move(o.dst_);
  type_ = std::move(o.type_);
  desc_ = std::move(o.desc_);
  data_ = std::move(o.data_);
  any_data_ = std::move(o.any_data_);
  return *this;
}

Msg& Msg::operator=(const Msg& o) noexcept {
  trans_mode_ = o.trans_mode_;
  src_ = o.src_;
  dst_ = o.dst_;
  type_ = o.type_;
  desc_ = o.desc_;
  data_ = o.data_;
  any_data_ = o.any_data_;
  return *this;
}

std::ostream& operator<<(std::ostream& out, const Msg& msg) {
  out << "[" << msg.GetSrc()
    << " to " << msg.GetDst()
    << "] trans mode: " << static_cast<int>(msg.GetTransMode())
    << ", type: " << msg.GetType();
  return out;
}

}  // namespace myframe

