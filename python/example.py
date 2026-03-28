#!/usr/bin/python3
# -*- coding: UTF-8 -*-
"""
执行前请设置好环境变量
source setup.sh
"""
import time
import pymyframe as myframe

# 创建actor对象接收主线程发送的消息
class Actor:
    def init(self):
        print(f"actor init")
        return 0

    # 接收 pymyframe.Msg
    def proc(self, msg):
        print(f"{msg.getDst()} get msg from {msg.getSrc()}: {msg.getData()}")

# 初始化
app = myframe.App()
app_conf = myframe.AppConf()
res = app.init(app_conf)
if res == False:
    exit(-1)

# 加载service目录中的所有组件
# res = app.loadServiceFromDir("service")
# if res == False:
#     exit(-1)

# 加载service目录中的单个组件
# res = app.loadServiceFromFile("service/example_actor_timer.json")
# if res == False:
#     exit(-1)

# 添加actor对象到框架中
# 注意不要销毁actor对象否则会导致调用异常
actor = Actor()
res = app.addActor(actor,
    """
    {
        "instance_name":"0"
    }
    """
)
if res == False:
    exit(-1)

app.start()

while True:
    time.sleep(1)

    msg_text = "hello,world"
    print(f"mainthread send msg: {msg_text}")

    msg = myframe.Msg()
    msg.setDst("actor.PyActor.0")
    msg.setData(msg_text)
    msg.setTransMode(myframe.Msg.TransMode_INTRA)
    app.send(msg)
