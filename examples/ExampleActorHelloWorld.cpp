#include <glog/logging.h>

#include "myframe/MyActor.h"
#include "myframe/MyMsg.h"

/*
    自己给自己发送一条消息
*/
class ExampleActorHelloWorld : public MyActor
{
public:
    /* actor模块加载完毕后调用 */
    int Init(const char* param) override {
        /* 构造 hello,world 消息发送给自己 */
        return Send("actor.example.hello_world", std::make_shared<MyMsg>("hello,world"));
    }

    void CB(const std::shared_ptr<const MyMsg>& msg) override {
        /* 获得文本消息， 打印 源actor地址 目的actor地址 消息内容*/
        LOG(INFO) << "----> from \"" << msg->GetSrc() << "\" to \"" 
            << GetActorName() << "\": " << msg->GetData();
    }
};

/* 创建actor模块实例函数 */
extern "C" std::shared_ptr<MyActor> my_actor_create(const std::string& actor_name) {
    return std::make_shared<ExampleActorHelloWorld>();
}
