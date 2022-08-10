#pragma once
#include <stdint.h>
#include <stddef.h>

#include <memory>
#include <string>

/* 系统的句柄号 */
#define MY_FRAME_DST 0xffffff
#define MY_FRAME_DST_NAME "myframe"

class MyMsg;
class MyContext;

/**
 * MyModule - 服务模块
 * 
 * 编写的服务都需要继承该类，并重写Init()/CB()方法
 * 
 */
class MyActor
{
    friend class MyApp;
    friend class MyContext;
    friend class MyModLib;
public:
    MyActor();
    virtual ~MyActor();

    /**
     * Init() - 服务初始化调用的初始化函数
     * @c:      服务指针
     * @param:  服务参数
     * 
     * @return: 未定义
     */
    virtual int Init(const char* param) = 0;

    /**
     * CB() - 回调函数
     * @msg:      服务收到的消息
     * 
     */
    virtual void CB(std::shared_ptr<MyMsg>& msg) = 0;

    /**
     * Send() - 发送消息给别的服务
     * @dst:            目的服务
     * @msg:            发送的消息
     * 
     *      将消息添加到该服务的消息发送队列中，等待服务执行完成后，myframe会将消息分发给其他服务
     * 
     * @return:         成功 0， 失败 -1
     */
    int Send(const std::string& dst, std::shared_ptr<MyMsg> msg);

    /**
     * GetHandle() - 获得该服务的句柄号
     * 
     *      服务句柄主要用于在给另一个服务发送消息时，指定另一个服务的句柄号时会用到
     * 
     * @return:         服务句柄
     */
    uint32_t GetHandle();

    /**
     * GetServiceName() - 获得该服务的服务名
     * 
     * @return:         成功返回：服务名，失败返回：空字符串
     */
    std::string GetServiceName();

    /**
     * Timeout() - 设置定时器
     * @time:           超时时间(单位:10ms, 比如 time = 1, 那么超时时间就是10ms)
     * 
     *      定时器设置之后，过了超时时间，服务就会收到超时消息;
     *      如果想实现周期性的定时器，可以在收到超时消息之后，
     *      再次调用此函数设置下一次的超时。
     * 
     * @return:         成功返回: 0, 失败返回: -1
     */
    int Timeout(int time);

private:
    bool IsFromLib() { return _is_from_lib; }
    void SetContext(MyContext*);
    bool _is_from_lib = false;
    std::string m_mod_name;
    std::string m_service_name;
    std::string m_instance_name;
    MyContext*  m_ctx;
};

extern "C" {
    typedef std::shared_ptr<MyActor> (*my_actor_create_func)(const std::string&);
} // extern "C"
