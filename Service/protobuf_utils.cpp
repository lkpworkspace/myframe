#include "protobuf_utils.h"
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

///////////////////////////////////////////////////
/// MyRpcController
///////////////////////////////////////////////////
void MyRpcController::Reset()
{}

bool MyRpcController::Failed() const
{}

std::string MyRpcController::ErrorText() const
{}

void MyRpcController::StartCancel()
{}

void MyRpcController::SetFailed(const std::string& reason)
{}

bool MyRpcController::IsCanceled() const
{}

void MyRpcController::NotifyOnCancel(::google::protobuf::Closure* callback)
{}

///////////////////////////////////////////////////
/// MyRpcServer
///////////////////////////////////////////////////
void MyRpcServer::Add(::google::protobuf::Service* service)
{
    ServiceInfo service_info;
    service_info.service = service;
    service_info.sd = service->GetDescriptor();
    for (int i = 0; i < service_info.sd->method_count(); ++i) {
        service_info.mds[service_info.sd->method(i)->name()] = service_info.sd->method(i);
    }

    m_services[service_info.sd->name()] = service_info;
}

void MyRpcServer::Start(const std::string& ip, const int port)
{
    // listen
    // TODO...
}

// 消息处理
void MyRpcServer::DispatchMsg(
        uint32_t id,
        const std::string& service_name,
        const std::string& method_name,
        const std::string& serialzied_data)
{
    m_sock_id = id;
    auto service = m_services[service_name].service;
    auto md = m_services[service_name].mds[method_name];

    // std::cout << "recv service_name:" << service_name << std::endl;
    // std::cout << "recv method_name:" << method_name << std::endl;
    // std::cout << "recv type:" << md->input_type()->name() << std::endl;
    // std::cout << "resp type:" << md->output_type()->name() << std::endl;

    auto recv_msg = service->GetRequestPrototype(md).New();
    recv_msg->ParseFromString(serialzied_data);
    // std::cout << "requ msg: " << recv_msg->DebugString() << std::endl;
    auto resp_msg = service->GetResponsePrototype(md).New();

    MyRpcController controller;
    auto done = ::google::protobuf::NewCallback(
                this,
                &MyRpcServer::RespCB,
                recv_msg,
                resp_msg);
    service->CallMethod(md, &controller, recv_msg, resp_msg, done);
}

void MyRpcServer::RespCB(
        ::google::protobuf::Message* recv_msg,
        ::google::protobuf::Message* resp_msg) {
    //avoid mem leak
    boost::scoped_ptr<::google::protobuf::Message> recv_msg_guard(recv_msg);
    boost::scoped_ptr<::google::protobuf::Message> resp_msg_guard(resp_msg);

    std::string resp_str;
    PackMessage(resp_msg, &resp_str);
    // std::cout << "respon str: " << resp_str << std::endl;
    //my_sock_send(m_sock_id, resp_str.data(), resp_str.size());
    m_send.push_back(resp_str);
}

void MyRpcServer::PackMessage(
        const ::google::protobuf::Message* msg,
        std::string* serialized_data) {
    int serialized_size = msg->ByteSize();
    serialized_data->assign(
                (const char*)&serialized_size,
                sizeof(serialized_size));
    msg->AppendToString(serialized_data);
}


google::protobuf::Message*
my_create_proto_message(const std::string& type_name)
{
    google::protobuf::Message* message = NULL;
    const google::protobuf::Descriptor* descriptor =
            google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
    if (descriptor)
    {
        const google::protobuf::Message* prototype =
                google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if (prototype)
        {
            message = prototype->New();
        }
    }
    return message;
}
