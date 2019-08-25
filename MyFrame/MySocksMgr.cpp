#include "MySocksMgr.h"

#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <cstring>
#include <boost/log/trivial.hpp>

#include "MySock.h"
#include "MyContext.h"
#include "MyApp.h"
#include "MyCUtils.h"

union sockaddr_all{
    struct sockaddr s;
    struct sockaddr_in v4;
    struct sockaddr_in6 v6;
};

MySocksMgr::MySocksMgr() :
    m_alloc_id(0)
{
    pthread_rwlock_init(&m_rw, NULL);
    m_ids.clear();
}

MySocksMgr::~MySocksMgr()
{
    pthread_rwlock_destroy(&m_rw);
}

// 申请ID在极端情况下会申请失败
// TODO...
uint32_t MySocksMgr::RegId(MySock* sock)
{
    uint32_t id;
    pthread_rwlock_wrlock(&m_rw);
    id = m_alloc_id;
    if(m_ids.end() != m_ids.find(id)){
        BOOST_LOG_TRIVIAL(error) << "Alloc id:" << id << " failed";
        exit(-1);
    }
    m_ids[m_alloc_id] = sock;
    m_alloc_id++;
    pthread_rwlock_unlock(&m_rw);
    return id;
}

void MySocksMgr::UnregId(MySock* sock)
{
    UnregId(sock->m_id);
}

void MySocksMgr::UnregId(uint32_t id)
{
    pthread_rwlock_wrlock(&m_rw);
    if(m_ids.end() != m_ids.find(id)){
        m_ids.erase(id);
    }
    pthread_rwlock_unlock(&m_rw);
}

MySock* MySocksMgr::GetSock(uint32_t id)
{
    MySock* sock = nullptr;
    pthread_rwlock_rdlock(&m_rw);
    if(m_ids.end() != m_ids.find(id))
        sock = m_ids[id];
    pthread_rwlock_unlock(&m_rw);
    return sock;
}

int MySocksMgr::DoBind(const char* host, int port, int protocol, int* family)
{
    int fd;
    int status;
    int reuse = 1;
    struct addrinfo ai_hints;
    struct addrinfo *ai_list = NULL;
    char portstr[16];
    if(host == NULL || host[0] == 0){
        host = "0.0.0.0"; // INADDR_ANY
    }
    sprintf(portstr, "%d", port);
    std::memset(&ai_hints, 0, sizeof(ai_hints));
    ai_hints.ai_family = AF_UNSPEC;
    if(protocol == IPPROTO_TCP){
        ai_hints.ai_socktype = SOCK_STREAM;
    }else{
        assert(protocol == IPPROTO_UDP);
        ai_hints.ai_socktype = SOCK_DGRAM;
    }
    ai_hints.ai_protocol = protocol;

    status = getaddrinfo(host, portstr, &ai_hints, &ai_list);
    if(status != 0) return -1;
    *family = ai_list->ai_family;
    fd = socket(*family, ai_list->ai_socktype, 0);
    if(fd < 0) {
        freeaddrinfo(ai_list);
        return -1;
    }
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&reuse, sizeof(int)) == -1){
        close(fd);
        freeaddrinfo(ai_list);
        return -1;
    }
    status = bind(fd, (struct sockaddr*)ai_list->ai_addr, ai_list->ai_addrlen);
    if(status != 0){
        close(fd);
        freeaddrinfo(ai_list);
        return -1;
    }
    return fd;
}

int MySocksMgr::DoListen(const char* host, int port, int backlog)
{
    int family = 0;
    int listen_fd = DoBind(host, port, IPPROTO_TCP, &family);
    if(listen_fd < 0){
        return -1;
    }
    if(listen(listen_fd, backlog) == -1){
        close(listen_fd);
        return -1;
    }
    return listen_fd;
}

int MySocksMgr::Listen(MyContext* ctx, const char* addr, int port, int backlog)
{
    int fd = DoListen(addr, port, backlog);
    if (fd < 0) {
        return -1;
    }
    // create socket
    MySock* sock = NewSock(ctx->m_handle, fd, MySock::PROTOCOL_TCP);
    sock->m_state = MySock::SOCK_STATE_LISTEN;
    MyApp::Inst()->AddEvent(sock);
    return sock->m_id;
}

int MySocksMgr::Connect(MyContext* ctx, const char* addr, int port)
{
    // TODO...
    return 0;
}

int MySocksMgr::Bind(MyContext* ctx, int fd)
{
    // TODO...
    return 0;
}

MySock* MySocksMgr::Accept(MySock* sock)
{
    MySock* client_sock;
    int client_fd;
    union sockaddr_all u;
    socklen_t len = sizeof(u);
    // ACCEPT
    client_fd = accept(sock->m_fd, &u.s, &len);
    if(client_fd < 0){
        return nullptr;
    }
    // keepalive
    int keepalive = 1;
    setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive , sizeof(keepalive));
    // set nonblocking
    my_set_nonblock(client_fd, true);
    // NewSock
    client_sock = NewSock(sock->m_handle, client_fd, MySock::PROTOCOL_TCP);
    client_sock->m_state = MySock::SOCK_STATE_CONNECTED;
    MyApp::Inst()->AddEvent(client_sock);
    // TODO...
    return client_sock;
}

MySock* MySocksMgr::NewSock(uint32_t handle, int fd, int protocol)
{
    MySock* sock = new MySock();
    uint32_t id = RegId(sock);
    sock->m_fd = fd;
    sock->m_type = protocol;
    sock->m_id = id;
    sock->m_handle = handle;
    return sock;
}

void MySocksMgr::Close(uint32_t id)
{
    MySock* s = GetSock(id);
    // 注销ID
    UnregId(id);
    // 发送未发送的数据
    s->Close();
    // 删除
    delete s;
}
