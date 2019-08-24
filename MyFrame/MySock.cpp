#include "MySock.h"

#include <string.h>
#include <unistd.h>

#include <boost/log/trivial.hpp>

#include "MySocksMgr.h"
#include "MyApp.h"
#include "MyFrame.h"
#include "MyMsg.h"
#include "MyCUtils.h"

#define WARNING_SIZE (1024*1024)
#define MIN_READ_BUFFER 64

MySock::MySock() :
    m_rd_size(MIN_READ_BUFFER),
    m_wb_size(0),
    m_warn_size(WARNING_SIZE),
    m_state(SOCK_STATE_INVALID),
    m_type(PROTOCOL_UNKNOWN),
    m_epoll_events(0),
    m_epoll_flag(EPOLLIN),
    m_fd(-1),
    m_id(-1),
    m_handle(-1)
{}

MySock::~MySock()
{}

int MySock::GetEventType()
{
    return EV_SOCK;
}

void MySock::SetEpollWrite(bool b)
{
    if(b){
        m_epoll_flag |= EPOLLOUT;
    }else{
        m_epoll_flag &= ~EPOLLOUT;
    }
}

unsigned int MySock::GetEpollEventType()
{
    return m_epoll_flag;
}

// 暂时不做写缓存处理
// TODO..
int MySock::Send(const void* buffer, int sz)
{
    int ret = write(m_fd, buffer, sz);
    if(ret == -1)
        BOOST_LOG_TRIVIAL(error) << my_get_error();
    return ret;
}

void MySock::SendWriteList()
{

}

MyList *MySock::CB(MyEvent* ev, int* ud)
{
    ev = ev;
    // 判断该sock类型
    // 判断待处理事件类型()
    // 处理消息
    // 生成回复消息
    // 加入发送队列
    *ud = 1;
    MySockMsg* smsg = new MySockMsg();

    smsg->source = MY_FRAME_DST;
    smsg->destination = m_handle;
    switch(m_state){
    case SOCK_STATE_LISTEN: { // 有新的客户端连接
        MySock* client_sock = MyApp::Inst()->GetSocksMgr()->Accept(this);
        if(client_sock == nullptr){
            BOOST_LOG_TRIVIAL(error) << "Accept error";
            return &m_send;
        }else{
            BOOST_LOG_TRIVIAL(debug) << "New connect " << client_sock->m_id;
        }
        smsg->SetSockId(client_sock->m_id);
        smsg->SetSockMsgType(MySockMsg::MySockMsgType::ACCEPT);

        m_send.AddTail(smsg);
        break;
    }
    case SOCK_STATE_CONNECTED: { // 已经连接的客户端
        // 数据可读事件
        if(m_epoll_events & (EPOLLIN | EPOLLHUP)){
            MySockMsg::MySockMsgType socket_type = MySockMsg::MySockMsgType::DATA;
            int sz = m_rd_size;
            char * buffer = (char*)malloc(sz);
            int n = (int)read(m_fd, buffer, sz);
            if (n<0) {
                free(buffer);
                buffer = nullptr;
                switch(errno) {
                case EINTR:
                    BOOST_LOG_TRIVIAL(debug) << "socket-server: read EINTR capture";
                    break;
                case EAGAIN:
                    BOOST_LOG_TRIVIAL(debug) << "socket-server: read EAGAIN capture";
                    break;
                default:
                    BOOST_LOG_TRIVIAL(error) << "socket-server: read error: " << errno;
                    // close socket when error
                    // 发送错误消息给服务
                    // TODO...
                    socket_type = MySockMsg::MySockMsgType::ERROR;
                    *ud = 0;
                    break;
                }
                return &m_send;
            }
            if (n==0) {
                BOOST_LOG_TRIVIAL(debug) << "Sock id:" << m_id << " read 0 byte";
                free(buffer);
                buffer = nullptr;
                // close socket
                // TODO...
                socket_type = MySockMsg::MySockMsgType::CLOSE;
                *ud = 0;
                m_send.AddTail(CloseLater());
            }

            if (m_state == SOCK_STATE_HALFCLOSE) {
                BOOST_LOG_TRIVIAL(debug) << "Sock id:" << m_id << " HALFCLOSE";
                // discard recv data
                free(buffer);
                buffer = nullptr;
                return &m_send;
            }
            if (n == sz) {
                m_rd_size *= 2;
                BOOST_LOG_TRIVIAL(debug) << "Sock id:" << m_id << "rd size change " << m_rd_size;
            } else if (sz > MIN_READ_BUFFER && n*2 < sz) {
                m_rd_size /= 2;
                BOOST_LOG_TRIVIAL(debug) << "Sock id:" << m_id << "rd size change " << m_rd_size;
            }
            BOOST_LOG_TRIVIAL(debug) << "Sock id:" << m_id << " get msg type:" << (int)socket_type;

            if(buffer != nullptr){
                smsg->SetData(buffer, n);
            }
            smsg->SetSockId(m_id);
            smsg->SetSockMsgType(socket_type);
           
            m_send.AddTail(smsg);
        }
        // 数据可写事件
        if(m_epoll_events & EPOLLOUT){
            // 目前没用
            // TODO...
        }
        // 错误事件
        if(m_epoll_events & EPOLLERR){
            // TODO...
        }
        break;
    }
    case SOCK_STATE_CONNECTING: // 客户端还在连接中
        // TODO...
        break;
    default:
        break;
    }
    return &m_send;
}

// 该函数是构造一个发送给系统的消息，并没有发送给服务
MyMsg* MySock::CloseLater()
{
    MySockMsg* smsg = new MySockMsg();
    smsg->source = m_handle;
    smsg->destination = MY_FRAME_DST;

    smsg->SetSockId(m_id);
    smsg->SetSockMsgType(MySockMsg::MySockMsgType::CLOSE);
    return smsg;
}
