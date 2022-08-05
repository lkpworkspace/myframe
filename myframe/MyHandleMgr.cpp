#include "MyHandleMgr.h"
#include "MyContext.h"
#include "assert.h"

#include <string.h>

#include "MyLog.h"

#define MY_DEFAULT_SLOT_SIZE 4

MyHandleMgr::MyHandleMgr() :
    m_harbor(0),
    m_slot_size(MY_DEFAULT_SLOT_SIZE),
    m_slot_idx(0),
    m_ctx_count(0),
    m_handle_index(0)
{
    pthread_rwlock_init(&m_rw, NULL);
    m_slot = (MyContext**)malloc(sizeof(MyContext*) * m_slot_size);
    memset(m_slot, 0, sizeof(MyContext*) * m_slot_size);
}

MyHandleMgr::~MyHandleMgr()
{
    pthread_rwlock_destroy(&m_rw);
}

uint32_t MyHandleMgr::RegHandle(MyContext* ctx)
{
    pthread_rwlock_wrlock(&m_rw);
    for (;;) {
        int i;
        for (i = 0; i < m_slot_size; i++) {
            uint32_t handle = (i + m_handle_index) & MY_HANDLE_MASK;
            int hash = handle & (m_slot_size-1);
            if (m_slot[hash] == NULL) {
                m_slot[hash] = ctx;
                m_handle_index = handle + 1;

                handle |= m_harbor;
                ctx->m_handle = handle;
                if(m_named_ctxs.find(ctx->m_mod->m_service_name) == m_named_ctxs.end()){
                    m_named_ctxs[ctx->m_mod->m_service_name] = handle;
                }else{
                    LOG(WARNING) << "reg the same service name: " << ctx->m_mod->m_service_name;
                }
                m_ctx_count++;
                pthread_rwlock_unlock(&m_rw);
                return handle;
            }
        }
        assert((m_slot_size * 2 - 1) <= MY_HANDLE_MASK);
        MyContext** new_slot = (MyContext**)malloc(m_slot_size * 2 * sizeof(MyContext*));
        memset(new_slot, 0, m_slot_size * 2 * sizeof(MyContext*));
        for (i = 0; i < m_slot_size; i++) {
            int hash = m_slot[i]->GetHandle() & (m_slot_size * 2 - 1);
            assert(new_slot[hash] == NULL);
            new_slot[hash] = m_slot[i];
        }
        free(m_slot);
        m_slot = new_slot;
        m_slot_size *= 2;
    }
    pthread_rwlock_unlock(&m_rw);
    return 0;
}

MyContext* MyHandleMgr::GetContext(std::string& service_name)
{
    uint32_t handle = 0x00;
    pthread_rwlock_rdlock(&m_rw);
    if(m_named_ctxs.find(service_name) != m_named_ctxs.end()){
        handle = m_named_ctxs[service_name];
        pthread_rwlock_unlock(&m_rw);
        return GetContext(handle);
    }
    pthread_rwlock_unlock(&m_rw);
    return nullptr;
}

MyContext* MyHandleMgr::GetContext(uint32_t handle)
{
    MyContext* result = nullptr;
    pthread_rwlock_rdlock(&m_rw);
    uint32_t hash = handle & (m_slot_size-1);
    MyContext* ctx = m_slot[hash];
    if (ctx && ctx->m_handle == handle) {
        result = ctx;
    }
    pthread_rwlock_unlock(&m_rw);
    return result;
}

MyContext* MyHandleMgr::GetContext(bool onethread)
{
    MyList& msg_list = onethread ? m_imsg_list : m_msg_list;
    if(msg_list.IsEmpty()) return nullptr;

    bool first = true;
    MyNode* b = msg_list.Begin();
    MyNode* ctx_node = nullptr; 
    MyContext* ctx = nullptr;

    while(!msg_list.IsEmpty()){
        ctx_node = msg_list.Begin();
        ctx = static_cast<MyContext*>(ctx_node);

        if(ctx->m_in_global == false){
            msg_list.DelHead();
            msg_list.AddTail(ctx_node);
        }else{
            msg_list.DelHead();

            ctx->m_in_msg_list = false;
            ctx->m_in_global = false;
            return ctx;
        }
        if((b == ctx_node) && !first){
            break;
        } 
        first = false;
    }
    return nullptr;
}

void MyHandleMgr::PushContext(MyContext* ctx)
{
    if(ctx->m_in_msg_list) return;
    ctx->m_in_msg_list = true;
    if(ctx->RunInOneThread())
        m_imsg_list.AddTail(ctx);
    else
        m_msg_list.AddTail(ctx);
}
