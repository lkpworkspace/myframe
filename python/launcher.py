#!/usr/bin/python3
# -*- coding: UTF-8 -*-
import time
import pymyframe as myframe

class Actor:
    def proc(self, msg):
        print(f"get msg {msg}")

app = myframe.App()
app_conf = myframe.AppConf()

res = app.init(app_conf)
if res == False:
    exit(-1)

res = app.loadServiceFromDir("service")
if res == False:
    exit(-1)

actor = Actor()
app.addActor(actor,
    """
    {
        "instance_name":"0"
    }
    """)

app.start()

while True:
    time.sleep(1)
    msg = "hello, world"
    print(f"mainthread send msg {msg}")
    msg = myframe.Msg("actor.PyActor.0", msg)
    app.send(msg)
