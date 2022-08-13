#include <iostream>
#include <string.h>

#include <glog/logging.h>

#include "myframe/MyActor.h"
#include "myframe/MyMsg.h"

/*
    该actor实现：
        自己给自己发送一条消息
*/
class MyDemo : public MyActor
{
public:
    /* actor模块加载完毕后调用 */
    int Init(const char* param) override {
        /* 构造 hello,world 消息发送给自己 */
        return Send("actor.demo.echo_hello_world", std::make_shared<MyTextMsg>("hello,world"));
    }

    void CB(const std::shared_ptr<const MyMsg>& msg) override {
        if (msg->GetMsgType() == "TEXT") {
            /* 获得文本消息， 打印 源actor地址 目的actor地址 消息内容*/
            const auto& tmsg = std::dynamic_pointer_cast<const MyTextMsg>(msg);
            LOG(INFO) << "----> from \"" << tmsg->GetSrc() << "\" to \"" 
                << GetActorName() << "\": " << tmsg->GetData();
        }
    }
};

/* 创建actor模块实例函数 */
extern "C" std::shared_ptr<MyActor> my_actor_create(const std::string& actor_name) {
    return std::make_shared<MyDemo>();
}
