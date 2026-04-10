#!/usr/bin/python3
# -*- coding: UTF-8 -*-

import time
import myframe_setup
import pymyframe as myframe

class TestPub(myframe.Actor):
    def __init__(self):
        super().__init__()
        self.num = 0

    def init(self):
        print(f"{self.getActorName()} init")
        self.timeout("pub", 1000)
        return 0

    def proc(self, msg):
        if msg.getType() == "TIMER":
            self.publish(f"hello, this is pub msg {self.num}")
            self.num += 1
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
app_conf.setLogDir("")
res = app.init(app_conf)
if res == False:
    exit(-1)

# 添加actor对象到框架中
# 注意不要销毁actor对象否则会导致调用异常

test_pub = TestPub()
res = app.addActor(test_pub,
    """
    {
        "instance_name":"test_pub"
    }
    """)
if res == False:
    exit(-1)

test_sub = TestSub()
res = app.addActor(test_sub,
    """
    {
        "instance_name":"test_sub"
    }
    """)
if res == False:
    exit(-1)

app.start()

while True:
    time.sleep(1)
