/****************************************************************************
Copyright (c) 2018, likepeng
All rights reserved.
 
Author: likepeng <likepeng0418@163.com>
****************************************************************************/

#include "MyActor.h"
#include "MyContext.h"
#include "MyWorkerTimer.h"
#include "MyApp.h"

namespace myframe {

MyActor::MyActor() {}

MyActor::~MyActor()
{}

void MyActor::SetModName(const std::string& name) {
    if (_mod_name == "class") {
        _is_from_lib = false;
    } else {
        _is_from_lib = true;
    }
    _mod_name = name;
}

int MyActor::Send(const std::string& dst, std::shared_ptr<MyMsg> msg) {
    if (_ctx.expired()) {
        return -1;
    }
    msg->SetSrc(GetActorName());
    msg->SetDst(dst);
    return _ctx.lock()->SendMsg(msg);
}

const std::string MyActor::GetActorName() const {
    return "actor." + _actor_name + "." + _instance_name;
}

int MyActor::Timeout(const std::string& timer_name, int expired) {
    if (_ctx.expired()) {
        return -1;
    }
    return _ctx.lock()->GetApp()->GetTimerWorker()->SetTimeout(GetActorName(), timer_name, expired);
}

void MyActor::SetContext(std::shared_ptr<MyContext> c) {
    _ctx = c;
}

} // namespace myframe