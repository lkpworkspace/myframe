#!/usr/bin/python3
# -*- coding: UTF-8 -*-

# 打印堆栈信息
# import faulthandler
# faulthandler.enable()

import time
import myframe_setup
import pymyframe as myframe

# 创建actor对象接收主线程发送的消息
class TestRecv(myframe.Actor):
    def init(self):
        print(f"{self.getActorName()} init")
        return 0

    def proc(self, msg):
        print(f"TestRecv: {msg.debugString()}, data: {msg.getData()}")

# 创建actor对象接收定时器消息
class TestTimer(myframe.Actor):
    def init(self):
        print(f"{self.getActorName()} init")
        self.timeout("timer", 1000)
        return 0

    def proc(self, msg):
        print(f"TestTimer: {msg.debugString()}, data: {msg.getData()}")
        self.timeout("timer", 1000)

# 初始化
app = myframe.App()
app_conf = myframe.AppConf()
app_conf.setLogDir("")
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

# res = app.loadServiceFromJsonStr(
#     """
#     {
#         "type":"library",
#         "lib":"example_actor_helloworld",
#         "actor":{
#             "ExampleActorHelloWorld":[
#                 {
#                     "instance_name":"1"
#                 }
#             ]
#         }
#     }
#     """)
# if res == False:
#     exit(-1)

# 添加actor对象到框架中
# 注意不要销毁actor对象否则会导致调用异常

test_recv = TestRecv()
res = app.addActor(test_recv,
    """
    {
        "instance_name":"test_recv"
    }
    """)
if res == False:
    exit(-1)

test_timer = TestTimer()
res = app.addActor(test_timer,
    """
    {
        "instance_name":"test_timer"
    }
    """)
if res == False:
    exit(-1)

app.start()

while True:
    time.sleep(1)

    msg_text = "hello,world"
    print(f"mainthread send msg: {msg_text}")

    msg = myframe.Msg()
    msg.setDst("actor.PyActor.test_recv")
    msg.setData(msg_text)
    msg.setTransMode(myframe.Msg.TransMode_INTRA)
    app.send(msg)
