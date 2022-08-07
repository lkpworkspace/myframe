#ifndef __MYHANDLEMGR_H__
#define __MYHANDLEMGR_H__
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <pthread.h>
#include "MyCommon.h"
#include "MyList.h"

#define MY_HANDLE_MASK 0xffffff
#define MY_HANDLE_REMOTE_SHIFT 24

class MyContext;
class MyHandleMgr
{
public:
    MyHandleMgr();
    virtual ~MyHandleMgr();

    /* 给服务分配句柄，并进行管理 */
    uint32_t RegHandle(MyContext* ctx);
    
    /* 获得句柄对应的服务 */
    MyContext* GetContext(uint32_t handle);

    /* 获得服务名对应的服务 */
    MyContext* GetContext(std::string& service_name);
    
    /* 获得一个待处理的服务 */
    MyContext* GetContext();

    /* 将有消息的服务放入链表 */
    void PushContext(MyContext* ctx);

private:
    /// 暂时没用
    uint32_t            m_harbor;
    /// 当前服务数组大小
    int                 m_slot_size;
    /// 当前要处理消息的服务下标
    uint32_t            m_slot_idx;
    /// 服务数组
    MyContext**         m_slot;
    /// 当前注册服务数量
    uint32_t            m_ctx_count;
    /// 分配的句柄
    uint32_t            m_handle_index;
    /// 待处理服务链表
    MyList              m_msg_list;
    /// 读写锁
    pthread_rwlock_t    m_rw;
    /// handle/name 映射表
    std::unordered_map<std::string, uint32_t> m_named_ctxs;

};

#endif
