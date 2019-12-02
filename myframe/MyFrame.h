#ifndef __MYFRAME_H__
#define __MYFRAME_H__

#include <cstddef>
#include <cstdint>
#include <string>

/* 系统的句柄号 */
#define MY_FRAME_DST 0xffffff

class MyMsg;
class MyContext;

/**
 * my_send() - 发送消息给别的服务
 * @ctx:            服务指针
 * @msg:            发送的消息
 * 
 *      将消息添加到该服务的消息发送队列中，等待服务执行完成后，myframe会将消息分发给其他服务
 * 
 * @return:         成功 0， 失败 -1
 */
int my_send(MyContext* ctx, MyMsg* msg);

/**
 * my_handle() - 由服务指针获得该服务的句柄号
 * @ctx:            服务指针
 * 
 *      服务句柄主要用于在给另一个服务发送消息时，指定另一个服务的句柄号时会用到
 * 
 * @return:         服务句柄
 */
uint32_t my_handle(MyContext* ctx);

/**
 * my_handle_name() - 由服务名获得该服务的句柄号
 * @service_name:   服务名称
 * 
 *      服务句柄主要用于在给另一个服务发送消息时，指定另一个服务的句柄号时会用到,
 *      服务名主要在配置文件中指定，每个服务名称不能同名，否则会有未知结果发生。
 * 
 * @return:         服务句柄
 */
uint32_t my_handle_name(std::string service_name);

/**
 * my_context() - 由服务句柄号获得该服务指针
 * @handle:         服务句柄号
 * 
 * @return:         成功返回：服务指针，失败返回：nullptr
 */
MyContext* my_context(uint32_t handle);

/**
 * my_context_name() - 由服务名获得该服务指针
 * @service_name:   服务名
 * 
 * @return:         成功返回：服务指针，失败返回：nullptr
 */
MyContext* my_context_name(std::string service_name);

/**
 * my_run_in_one_thread() - 指定该服务是否需要运行在一个单独的线程上
 * @ctx:            服务指针
 * @b:              true:服务运行在单独的线程上，false:服务运行在线程池中
 * 
 *      一些业务逻辑的上下文状态只能运行在单个线程上，可以设置该服务运行在单个线程上，
 *      在服务的Init()函数中调用此函数，设置服务运行方式。
 * 
 * @return:         void
 */
void my_run_in_one_thread(MyContext* ctx, bool b);

/**
 * my_timeout() - 设置定时器
 * @handle:         服务句柄
 * @time:           超时时间(单位:10ms, 比如 time = 1, 那么超时时间就是10ms)
 * @session:        设置定时器标识
 * 
 *      定时器设置之后，过了超时时间，服务就会收到超时消息;
 *      如果想实现周期性的定时器，可以在收到超时消息之后，
 *      再次调用此函数设置下一次的超时。
 * 
 * @return:         成功返回: session值, 失败返回: -1
 */
int my_timeout(uint32_t handle, int time, int session);

/**
 * my_cb() - 回调函数指针
 * @context:  服务指针
 * @msg:      服务收到的消息
 * @ud:       用户数据
 *  
 * @return:   未定义
 */
typedef int (*my_cb)(MyContext* context, MyMsg* msg, void* ud);
/**
 * my_callback() - 设置服务回调函数
 * @MyContext:      服务指针
 * @cb:             服务的回调函数指针
 * @ud:             用户数据
 * 
 *      此函数用于设置服务的消息处理函数，一般在在服务的Init()函数中调用此函数。
 * 
 * @return:         成功返回: session值, 失败返回: -1
 */
void my_callback(MyContext* ctx, my_cb cb, void* ud);

/**
 * my_listen() - 设置TCP服务器
 * @MyContext:      服务指针
 * @addr:           TCP的IP地址
 * @port:           监听的端口号
 * @backlog:        See listen()
 * 
 *      设置TCP监听后，该服务就会收到客户端的连接/退出/数据消息。
 * 
 * @return:         成功返回: 监听文件描述符, 失败返回: -1
 */
int my_listen(MyContext* ctx, const char* addr, int port, int backlog);

/**
 * my_sock_send() - 向TCP客户端发送数据
 * @id:            客户端连接id
 * @buf:           发送的数据
 * @sz:            数据长度
 * 
 *      设置TCP监听后，该服务就会收到客户端的连接/退出/数据消息。
 * 
 * @return:         成功返回: 监听文件描述符, 失败返回: -1
 */
int my_sock_send(uint32_t id, const void* buf, int sz);

#endif // __MYFRAME_H__
