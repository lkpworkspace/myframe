#include "MyFrame.h"
#include "MyApp.h"
#include "MyContext.h"
#include "MySock.h"
#include "MySocksMgr.h"

#include <string.h>

int my_send(MyContext* ctx,
            uint32_t source,
            uint32_t destination ,
            int type, /* 消息类型 */
            int session,
            void * msg, /* 消息内容 */
            size_t sz /* 消息长度 */)
{
    return ctx->SendMsg(source, destination, type, session, msg, sz);
}

void my_callback(MyContext* ctx, my_cb cb, void* ud)
{
    ctx->SetCB(cb, ud);
}

MyContext* my_context(uint32_t handle)
{
    MyContext* ctx = nullptr;
    ctx = MyApp::Inst()->GetContext(handle);
    return ctx;
}

uint32_t my_handle(MyContext* ctx)
{
    return ctx->GetHandle();
}

int my_listen(MyContext *ctx, const char *addr, int port, int backlog)
{
    return MyApp::Inst()->GetSocksMgr()->Listen(ctx, addr, port, backlog);
}

int my_sock_send(uint32_t id, const void *buf, int sz)
{
    return MyApp::Inst()->GetSocksMgr()->GetSock(id)->Send(buf,sz);
}
