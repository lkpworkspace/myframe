#pragma once
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <pthread.h>
#include "MyCommon.h"
#include "MyList.h"

#define MY_HANDLE_MASK 0xffffff
#define MY_HANDLE_REMOTE_SHIFT 24

class MyContext;
class MyHandleManager
{
public:
    MyHandleManager();
    virtual ~MyHandleManager();

    /* 给actor分配句柄，并进行管理 */
    uint32_t RegHandle(MyContext* ctx);
    
    /* 获得句柄对应的actor */
    MyContext* GetContext(uint32_t handle);

    /* 获得actor名对应的actor */
    MyContext* GetContext(const std::string& actor_name);
    
    /* 获得一个待处理的actor */
    MyContext* GetContextWithMsg();

    /* 将有消息的actor放入链表 */
    void PushContext(MyContext* ctx);

private:
    void PrintWaitQueue();
    
    /// 暂时没用
    uint32_t            m_harbor;
    /// 当前actor数组大小
    int                 m_slot_size;
    /// 当前要处理消息的actor下标
    uint32_t            m_slot_idx;
    /// actor数组
    MyContext**         m_slot;
    /// 当前注册actor数量
    uint32_t            m_ctx_count;
    /// 分配的句柄
    uint32_t            m_handle_index;
    /// 待处理actor链表
    MyList              m_msg_list;
    /// 读写锁
    pthread_rwlock_t    m_rw;
    /// handle/name 映射表
    std::unordered_map<std::string, uint32_t> m_named_ctxs;

};
