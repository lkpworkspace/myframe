#include <string.h>
#include <assert.h>

#include "MyHandleManager.h"
#include "MyContext.h"
#include "MyActor.h"
#include "MyLog.h"

#define MY_DEFAULT_SLOT_SIZE 4

MyHandleManager::MyHandleManager() :
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

MyHandleManager::~MyHandleManager()
{
    pthread_rwlock_destroy(&m_rw);
}

uint32_t MyHandleManager::RegHandle(MyContext* ctx)
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
                ctx->SetHandle(handle);
                if(m_named_ctxs.find(ctx->GetModule()->GetActorName()) == m_named_ctxs.end()){
                    m_named_ctxs[ctx->GetModule()->GetActorName()] = handle;
                }else{
                    LOG(WARNING) << "reg the same actor name: " << ctx->GetModule()->GetActorName();
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

MyContext* MyHandleManager::GetContext(const std::string& actor_name)
{
    uint32_t handle = 0x00;
    pthread_rwlock_rdlock(&m_rw);
    if(m_named_ctxs.find(actor_name) != m_named_ctxs.end()){
        handle = m_named_ctxs[actor_name];
        pthread_rwlock_unlock(&m_rw);
        return GetContext(handle);
    }
    pthread_rwlock_unlock(&m_rw);
    return nullptr;
}

MyContext* MyHandleManager::GetContext(uint32_t handle)
{
    MyContext* result = nullptr;
    pthread_rwlock_rdlock(&m_rw);
    uint32_t hash = handle & (m_slot_size-1);
    MyContext* ctx = m_slot[hash];
    if (ctx && ctx->GetHandle() == handle) {
        result = ctx;
    }
    pthread_rwlock_unlock(&m_rw);
    return result;
}

void MyHandleManager::PrintWaitQueue() {
    DLOG(INFO) << "cur wait queue actor:";
    auto begin = m_msg_list.Begin();
    while (begin != m_msg_list.End()) {
        auto next = begin->next;
        auto ctx = dynamic_cast<MyContext*>(begin);
        DLOG(INFO) << ctx->Print();
        begin = next;
    }
}

MyContext* MyHandleManager::GetContextWithMsg()
{
    MyList& msg_list = m_msg_list;
    if(msg_list.IsEmpty()) return nullptr;

    std::vector<MyContext*> in_runing_context;
    MyContext* ret = nullptr;
    while(!msg_list.IsEmpty()){
        auto ctx_node = msg_list.Begin();
        auto ctx = dynamic_cast<MyContext*>(ctx_node);

        if(ctx->IsRuning()){
            msg_list.DelHead();
            in_runing_context.push_back(ctx);
        }else{
            msg_list.DelHead();

            ctx->SetOutOfRunQueueFlag();
            ctx->SetRunFlag();
            ret = ctx;
            break;
        }
    }
    for (int i = 0; i < in_runing_context.size(); ++i) {
        DLOG(INFO) << in_runing_context[i]->GetModule()->GetActorName() << " is runing, move to wait queue back";
        msg_list.AddTail(in_runing_context[i]);
    }
    return ret;
}

void MyHandleManager::PushContext(MyContext* ctx)
{
    if(ctx->IsInRunQueue()) {
        DLOG(INFO) << ctx->Print() << " already in wait queue, return";
        PrintWaitQueue();
        return;
    }
    ctx->SetInRunQueueFlag();
    m_msg_list.AddTail(ctx);
    PrintWaitQueue();
}
