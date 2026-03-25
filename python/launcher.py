#!/usr/bin/python3
# -*- coding: UTF-8 -*-
import time
import pymyframe as myframe

app = myframe.App()
app_conf = myframe.AppConf()

res = app.init(app_conf)
if res == False:
    exit(-1)

res = app.loadServiceFromDir("service")
if res == False:
    exit(-1)

app.start()

while True:
    time.sleep(1)
