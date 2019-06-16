#ifndef __MYFRAME_H__
#define __MYFRAME_H__

#include <stddef.h>
#include <stdint.h>

/* 一级消息类型 */
#define MY_PTYPE_TEXT 0
#define MY_PTYPE_RESPONSE 1
#define MY_PTYPE_SOCKET 6
#define MY_PTYPE_TAG_DONTCOPY 0x10000
#define MY_PTYPE_TAG_ALLOCSESSION 0x20000

/* 二级消息类型 */
#define MY_SOCKET_TYPE_DATA 1
#define MY_SOCKET_TYPE_CONNECT 2
#define MY_SOCKET_TYPE_CLOSE 3
#define MY_SOCKET_TYPE_ACCEPT 4
#define MY_SOCKET_TYPE_ERROR 5
#define MY_SOCKET_TYPE_UDP 6
#define MY_SOCKET_TYPE_WARNING 7

#define MY_FRAME_DST 0xffffff

struct my_sock_msg{
    int type;
    int id;
    int ud;
    char * buffer;
};

class MyContext;
#ifdef __cplusplus
extern "C" {
#endif

/**
 * my_send() - 发送消息
 * @ctx:            服务指针
 * @source:         发送服务的handle
 * @destination:    发送的目的服务
 *                  发送给系统的消息可以使用 MY_FRAME_DST 宏
 * @type:           消息类型
 *                  MY_PTYPE_SOCKET
 *                  MY_PTYPE_TEXT
 * @session:        流水号
 * @msg:            消息指针
 * @sz:             消息长度
 *
 * @return:         成功 0， 失败 -1
 */
int my_send(MyContext* ctx,
         uint32_t source,
         uint32_t destination,
         int type, /* 消息类型 */
         int session,
         void * msg,/* 消息内容 */
         size_t sz /* 消息长度 */);

uint32_t my_handle(MyContext* ctx);

int my_timeout(uint32_t handle, int time, int session);

MyContext* my_context(uint32_t handle);

typedef int (*my_cb)(MyContext* context, void *ud, int type, int session, uint32_t source , const void *msg, size_t sz);
void my_callback(MyContext* ctx, my_cb cb, void* ud);

int my_listen(MyContext* ctx, const char* addr, int port, int backlog);
int my_sock_send(uint32_t id, const void* buf, int sz);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __MYFRAME_H__
