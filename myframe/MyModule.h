#ifndef __MYMODULE_H__
#define __MYMODULE_H__
#include <stdint.h>
#include <stddef.h>

#include <memory>
#include <string>

/* 系统的句柄号 */
#define MY_FRAME_DST 0xffffff

class MyMsg;
class MyContext;

/**
 * MyModule - 服务模块
 * 
 * 编写的服务都需要继承该类，并重写Init()/CB()方法
 * 
 */
class MyModule
{
    friend class MyContext;
    friend class MyModLib;
    friend class MyHandleMgr;
public:
    MyModule();
    MyModule(const std::string& mod_name, const std::string& service_name);

    virtual ~MyModule();

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
     * @return:   1: 由myframe删除msg消息， 0: 由服务删除msg消息
     */
    virtual int CB(MyMsg* msg) = 0;

    /**
     * Send() - 发送消息给别的服务
     * @msg:            发送的消息
     * 
     *      将消息添加到该服务的消息发送队列中，等待服务执行完成后，myframe会将消息分发给其他服务
     * 
     * @return:         成功 0， 失败 -1
     */
    int Send(MyMsg* msg);

    /**
     * GetHandle() - 获得该服务的句柄号
     * 
     *      服务句柄主要用于在给另一个服务发送消息时，指定另一个服务的句柄号时会用到
     * 
     * @return:         服务句柄
     */
    uint32_t GetHandle();

    /**
     * my_handle_name() - 由服务名获得该服务的句柄号
     * @service_name:   服务名称
     * 
     *      服务句柄主要用于在给另一个服务发送消息时，指定另一个服务的句柄号时会用到,
     *      服务名主要在配置文件中指定，每个服务名称不能同名，否则会有未知结果发生。
     * 
     * @return:         成功返回: 服务句柄, 失败返回: -1
     */
    uint32_t GetHandle(std::string service_name);

    /**
     * GetServiceName() - 获得该服务的服务名
     * 
     * @return:         成功返回：服务名，失败返回：空字符串
     */
    std::string GetServiceName();

    /**
     * GetServiceName() - 获得服务句柄对应的服务名
     * @handle:         服务句柄
     * 
     * @return:         成功返回：服务名，失败返回：空字符串
     */
    std::string GetServiceName(uint32_t handle);

    /**
     * CreateService() - 注册服务
     * @mod_inst:         模块动态库名
     * @service_name:     服务名
     * @params:           传递给该服务的参数
     * 
     *      建议不要在A服务中再注册A服务，容易形成递归注册导致程序崩溃。
     * 
     * @return:         成功返回：服务句柄，失败返回：0
     */
    uint32_t CreateService(std::string mod_name, std::string service_name, const char* params);

    /**
     * RunInOneThread() - 指定该服务是否需要运行在一个单独的线程上
     * @b:              true:服务运行在单独的线程上，false:服务运行在线程池中
     * 
     *      一些业务逻辑的上下文状态只能运行在单个线程上，可以设置该服务运行在单个线程上，
     *      在服务的Init()函数中调用此函数，设置服务运行方式。
     * 
     * @return:         void
     */
    void SetRunInOneThread(bool b = false);

    /**
     * Timeout() - 设置定时器
     * @time:           超时时间(单位:10ms, 比如 time = 1, 那么超时时间就是10ms)
     * @session:        设置定时器标识
     * 
     *      定时器设置之后，过了超时时间，服务就会收到超时消息;
     *      如果想实现周期性的定时器，可以在收到超时消息之后，
     *      再次调用此函数设置下一次的超时。
     * 
     * @return:         成功返回: session值, 失败返回: -1
     */
    int Timeout(int time, int session);

    /**
     * Listen() - 设置TCP服务器
     * @addr:           TCP的IP地址
     * @port:           监听的端口号
     * @backlog:        See listen()
     * 
     *      设置TCP监听后，该服务就会收到客户端的连接/退出/数据消息。
     * 
     * @return:         成功返回: 监听文件描述符, 失败返回: -1
     */
    int Listen(const char* addr, int port, int backlog);

    /**
     * SockSend() - 向TCP客户端发送数据
     * @id:            客户端连接id
     * @buf:           发送的数据
     * @sz:            数据长度
     * 
     *      设置TCP监听后，该服务就会收到客户端的连接/退出/数据消息。
     * 
     * @return:         成功返回: 监听文件描述符, 失败返回: -1
     */
    int SockSend(uint32_t id, const void* buf, int sz);

private:
    void SetContext(MyContext*);
    bool _is_from_lib = false;
    std::string m_mod_name;
    std::string m_service_name;
    std::string m_instance_name;
    MyContext*  m_ctx;
};

extern "C" {
    typedef std::shared_ptr<MyModule> (*my_mod_create_func)(const std::string&);
} // extern "C"

#endif
