/****************************************************************************
Copyright (c) 2019, 李柯鹏
All rights reserved.

Author: 李柯鹏 <likepeng0418@163.com>
****************************************************************************/
#include <iostream>
#include "myframe/msg.h"
#include "myframe/actor.h"
#include "myframe/mod_manager.h"
#include "myframe/app.h"

#include "performance_test_config.h"

class Hello : public myframe::Actor {
 public:
  /* actor模块加载完毕后调用 */
  int Init(const char* param) override {
    /* 构造 hello,world 消息发送给自己 */
    auto mailbox = GetMailbox();
    mailbox->Send(
      mailbox->Addr(),
      std::make_shared<myframe::Msg>("hello,world"));
    return 0;
  }

  void Proc(const std::shared_ptr<const myframe::Msg>& msg) override {
    /* 获得文本消息， 打印 源actor地址 目的actor地址 消息内容 */
    std::cout << *msg << ": " << msg->GetData() << std::endl;
  }
};

int main() {
  auto lib_dir =
    myframe::Common::GetAbsolutePath(MYFRAME_LIB_DIR).string();

  auto app = std::make_shared<myframe::App>();
  if (false == app->Init(lib_dir, 1)) {
    std::cout << "Init failed\n";
    return -1;
  }

  auto& mod = app->GetModManager();

  mod->RegActor("Hello", [](const std::string&) {
      return std::make_shared<Hello>();
  });
  auto actor = mod->CreateActorInst("class", "Hello");
  app->AddActor("1", "", actor);

  return app->Exec();
}
