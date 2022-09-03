#pragma once
#include <stdint.h>
#include <stddef.h>

#include <memory>
#include <string>

class MyMsg;
class MyContext;
class MyActor
{
    friend class MyApp;
    friend class MyContext;
    friend class MyModLib;
    friend class MyModManager;
public:
    MyActor();
    virtual ~MyActor();

    /**
     * Init() - actor初始化调用的初始化函数
     * @c:      actor指针
     * @param:  actor参数
     * 
     * @return: 未定义
     */
    virtual int Init(const char* param) = 0;

    /**
     * CB() - 回调函数
     * @msg:      actor收到的消息
     * 
     */
    virtual void CB(const std::shared_ptr<const MyMsg>& msg) = 0;

    /**
     * Send() - 发送消息给别的actor
     * @dst:            目的actor, eg: actor.demo.hellow_world
     * @msg:            发送的消息
     * 
     *      将消息添加到该actor的消息发送队列中，等待actor执行完成后，myframe会将消息分发给其他actor
     * 
     * @return:         成功 0， 失败 -1
     */
    int Send(const std::string& dst, std::shared_ptr<MyMsg> msg);

    /**
     * GetHandle() - 获得该actor的句柄号
     * 
     *      actor句柄主要用于在给另一个actor发送消息时，指定另一个actor的句柄号时会用到
     * 
     * @return:         actor句柄
     */
    uint32_t GetHandle() const;

    /**
     * GetActorName() - 获得该actor的actor名
     * 
     * @return:         成功返回：actor名，失败返回：空字符串
     */
    const std::string GetActorName() const;
    const std::string& GetTypeName() const { return _actor_name; }
    const std::string& GetInstName() const { return _instance_name; }

    /**
     * Timeout() - 设置定时器
     * @expired: 超时时间(单位:10ms, 比如 expired = 1, 那么超时时间就是10ms)
     * 
     *      定时器设置之后，过了超时时间，actor就会收到超时消息;
     *      如果想实现周期性的定时器，可以在收到超时消息之后，
     *      再次调用此函数设置下一次的超时。
     * 
     *      msg->GetMsgType() == "TIMER" 确认是定时器消息
     *      msg->GetMsgDesc() == timer_name 确认是那个定时器消息
     * 
     * @return:         成功返回: 0, 失败返回: -1
     */
    int Timeout(const std::string& timer_name, int expired);

private:
    void SetModName(const std::string& name);
    void SetTypeName(const std::string& name) { _actor_name = name; }
    void SetInstName(const std::string& name) { _instance_name = name; }
    bool IsFromLib() { return _is_from_lib; }
    void SetContext(MyContext*);

    bool _is_from_lib = false;
    std::string _mod_name;
    std::string _actor_name;
    std::string _instance_name;
    MyContext*  m_ctx;
};

extern "C" {
    typedef std::shared_ptr<MyActor> (*my_actor_create_func)(const std::string&);
} // extern "C"
