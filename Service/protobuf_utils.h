#ifndef __PROTOBUF_UTILS_H__
#define __PROTOBUF_UTILS_H__
#include <map>
#include <queue>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>

#include "MyFrame.h"
#include "MyList.h"

enum class PROTOBUF_TYPE :unsigned short {
    MSG,
    RPC,
    NONE
};

class MyRpcController : public ::google::protobuf::RpcController
{
public:
    virtual void Reset() override final;

    virtual bool Failed() const override final;

    virtual std::string ErrorText() const override final;

    virtual void StartCancel() override final;

    virtual void SetFailed(const std::string& reason) override final;

    virtual bool IsCanceled() const override final;

    virtual void NotifyOnCancel(::google::protobuf::Closure* callback) override final;
};

class MyRpcServer
{
public:
    // 注册RPC服务
    void Add(::google::protobuf::Service* service);

    // 开始监听、获得消息、分发消息
    void Start(const std::string& ip, const int port);

    // 消息处理
    void DispatchMsg(
        uint32_t id,
        const std::string& service_name,
        const std::string& method_name,
        const std::string& serialzied_data);

    // 回复消息回调函数
    void RespCB(
            ::google::protobuf::Message* recv_msg,
            ::google::protobuf::Message* resp_msg);

    std::vector<std::string>& GetSendMsg() { return m_send; }

    // msg --> string
    void PackMessage(
            const ::google::protobuf::Message* msg,
            std::string* serialized_data);

private:
    struct ServiceInfo{
        ::google::protobuf::Service* service;
        const ::google::protobuf::ServiceDescriptor* sd;
        std::map<std::string, const ::google::protobuf::MethodDescriptor*> mds;
    };

    // service_name -> {Service*, ServiceDescriptor*, MethodDescriptor* []}
    std::map<std::string, ServiceInfo> m_services;

    uint32_t m_sock_id;
    std::vector<std::string> m_send;
};


google::protobuf::Message* my_create_proto_message(const std::string& type_name);

#endif
