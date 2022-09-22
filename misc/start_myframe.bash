#!/bin/bash
cd ${HOME}/myframe/log
LD_LIBRARY_PATH=${HOME}/myframe/lib nohup ${HOME}/myframe/bin/myframe_main &
