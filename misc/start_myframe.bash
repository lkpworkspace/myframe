#!/bin/bash
cd /opt/myframe/log
ulimit -c unlimited
echo "1" >/proc/sys/kernel/core_uses_pid
echo "/opt/myframe/log/core-%e-%p" >/proc/sys/kernel/core_pattern
LD_LIBRARY_PATH=/opt/myframe/lib nohup /opt/myframe/bin/myframe_main &
