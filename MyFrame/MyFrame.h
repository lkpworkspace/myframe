#ifndef __MYFRAME_H__
#define __MYFRAME_H__

#include <stddef.h>
#include <stdint.h>

#define MY_PTYPE_TEXT 0
#define MY_PTYPE_SOCKET 6
#define MY_PTYPE_TAG_DONTCOPY 0x10000
#define MY_PTYPE_TAG_ALLOCSESSION 0x20000

class MyContext;
class MyApp;

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*my_cb)(MyContext* context, void *ud, int type, int session, uint32_t source , const void *msg, size_t sz);

int my_send(MyContext* ctx,
         uint32_t source,
         uint32_t destination ,
         int type, /* 消息类型 */
         int session,
         void * msg,/* 消息内容 */
         size_t sz /* 消息长度 */);

void my_callback(MyContext* ctx, my_cb cb, void* ud);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __MYFRAME_H__
