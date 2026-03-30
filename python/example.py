#!/usr/bin/python3
# -*- coding: UTF-8 -*-
"""
执行前请设置好环境变量
source setup.sh
"""
import time
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
        self.timeout("a", 1000)
        return 0

    def proc(self, msg):
        print(f"TestTimer: {msg.debugString()}, data: {msg.getData()}")
        self.timeout("a", 1000)

# 创建actor对象订阅消息
class TestPub(myframe.Actor):
    def __init__(self):
        super().__init__()
        self.sub_list = []

    def init(self):
        print(f"{self.getActorName()} init")
        self.timeout("pub", 1000)
        return 0

    def proc(self, msg):
        print(f"TestPub: {msg.debugString()}, data: {msg.getData()}")
        if msg.getType() == "SUBSCRIBE":
            self.sub_list.append(msg.getSrc())

        if msg.getType() == "TIMER":
            for sub_addr in self.sub_list:
                msg = myframe.Msg()
                msg.setDst(sub_addr)
                msg.setData("pub msg")
                self.send(msg)
            self.timeout("pub", 1000)

class TestSub(myframe.Actor):
    def init(self):
        print(f"{self.getActorName()} init")
        self.subscribe("actor.PyActor.test_pub", "")
        return 0

    def proc(self, msg):
        print(f"TestSub: {msg.debugString()}, data: {msg.getData()}")

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

test_recv = TestRecv()
res = app.addActor(test_recv,
    """
    {
        "instance_name":"test_recv"
    }
    """
)
if res == False:
    exit(-1)

test_timer = TestTimer()
res = app.addActor(test_timer,
    """
    {
        "instance_name":"test_timer"
    }
    """
)
if res == False:
    exit(-1)

test_pub = TestPub()
res = app.addActor(test_pub,
    """
    {
        "instance_name":"test_pub"
    }
    """
)
if res == False:
    exit(-1)

test_sub = TestSub()
res = app.addActor(test_sub,
    """
    {
        "instance_name":"test_sub"
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
    msg.setDst("actor.PyActor.test_recv")
    msg.setData(msg_text)
    msg.setTransMode(myframe.Msg.TransMode_INTRA)
    app.send(msg)
