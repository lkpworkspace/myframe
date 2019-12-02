#include "MyFrame.h"
#include "MyApp.h"
#include "MyContext.h"
#include "MySock.h"
#include "MySocksMgr.h"
#include "MyTimerTask.h"

#include <string.h>

int my_send(MyContext* ctx, MyMsg* msg)
{
    return ctx->SendMsg(msg);
}

void my_callback(MyContext* ctx, my_cb cb, void* ud)
{
    ctx->SetCB(cb, ud);
}

uint32_t my_handle(MyContext* ctx)
{
    return ctx->GetHandle();
}

uint32_t my_handle_name(std::string service_name)
{
    MyContext* ctx = nullptr;
    ctx = MyApp::Inst()->GetContext(service_name);
    return (ctx == nullptr) ? -1 : ctx->GetHandle();
}

MyContext* my_context(uint32_t handle)
{
    MyContext* ctx = nullptr;
    ctx = MyApp::Inst()->GetContext(handle);
    return ctx;
}

MyContext* my_context_name(std::string service_name)
{
    MyContext* ctx = nullptr;
    ctx = MyApp::Inst()->GetContext(service_name);
    return ctx;
}

void my_run_in_one_thread(MyContext* ctx, bool b)
{
    ctx->SetRunInOneThread(b);
}

int my_timeout(uint32_t handle, int time, int session)
{
    return MyApp::Inst()->GetTimerTask()->SetTimeout(handle, time, session);
}

int my_listen(MyContext *ctx, const char *addr, int port, int backlog)
{
    return MyApp::Inst()->GetSocksMgr()->Listen(ctx, addr, port, backlog);
}

int my_sock_send(uint32_t id, const void *buf, int sz)
{
    return MyApp::Inst()->GetSocksMgr()->GetSock(id)->Send(buf,sz);
}
