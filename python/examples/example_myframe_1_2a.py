#!/usr/bin/python3
# -*- coding: UTF-8 -*-

import time
import myframe_setup
import pymyframe as myframe

class ExampleNodePub1(myframe.Actor):
    def __init__(self):
        super().__init__()
        self.num = 1

    def init(self):
        print(f"{self.getActorName()} init")
        self.timeout("pub", 1000)
        return 0

    def proc(self, msg):
        if msg.getType() == "TIMER":
            self.publish(
                f"hello, this is A1x pub msg {self.num}",
                "x",
                myframe.Msg.TransMode_INTRA)
            self.publish(
                f"hello, this is A1y pub msg {self.num}",
                "y",
                myframe.Msg.TransMode_INTRA)
            self.num += 1
            self.timeout("pub", 1000)

class ExampleNodeSub2(myframe.Actor):
    def init(self):
        print(f"{self.getActorName()} init")
        self.subscribe(
            "actor.PyActor.example_node_pub1",
            "x",
            myframe.Msg.TransMode_INTRA)
        return 0

    def proc(self, msg):
        print(f"ExampleNodeSub2: {msg.debugString()}, data: {msg.getData()}")

class ExampleNodeSub3(myframe.Actor):
    def init(self):
        print(f"{self.getActorName()} init")
        self.subscribe(
            "actor.PyActor.example_node_pub1",
            "y",
            myframe.Msg.TransMode_INTRA)
        return 0

    def proc(self, msg):
        print(f"ExampleNodeSub3: {msg.debugString()}, data: {msg.getData()}")

# 初始化
app = myframe.App()
app_conf = myframe.AppConf()
app_conf.setLogDir("")
res = app.init(app_conf)
if res == False:
    exit(-1)

# 添加actor对象到框架中
# 注意不要销毁actor对象否则会导致调用异常
example_node_pub1 = ExampleNodePub1()
res = app.addActor(example_node_pub1,
    """
    {
        "instance_name":"example_node_pub1"
    }
    """)
if res == False:
    exit(-1)

example_node_sub2 = ExampleNodeSub2()
res = app.addActor(example_node_sub2,
    """
    {
        "instance_name":"example_node_sub2"
    }
    """)
if res == False:
    exit(-1)

example_node_sub3 = ExampleNodeSub3()
res = app.addActor(example_node_sub3,
    """
    {
        "instance_name":"example_node_sub3"
    }
    """)
if res == False:
    exit(-1)

app.start()

while True:
    time.sleep(1)
