#!/usr/bin/env sh
MYFRAME_SCRIPT_PATH="${BASH_SOURCE[0]:-${(%):-%x}}"
MYFRAME_CURR_DIR="$(cd "$(dirname "$MYFRAME_SCRIPT_PATH")" && pwd)"
MYFRAME_PARENT_DIR="$(cd "$MYFRAME_CURR_DIR/.." && pwd)"

OS_TYPE=$(uname -s)
case "$OS_TYPE" in
    Darwin)
        export DYLD_LIBRARY_PATH="${MYFRAME_PARENT_DIR}/lib:${DYLD_LIBRARY_PATH}"
        ;;
    Linux)
        export LD_LIBRARY_PATH="${MYFRAME_PARENT_DIR}/lib:${LD_LIBRARY_PATH}"
        ;;
    *)
        echo "Unknown OS: $OS_TYPE"
        ;;
esac

export PYTHONPATH=${PYTHONPATH}:${MYFRAME_PARENT_DIR}/lib/python

# export GLOG_v=1
# export GLOG_vmodule="app=1,actor_context_manager=1,mod_manager=1,worker_context_manager=1"
# export GLOG_minloglevel=0
# export GLOG_logtostderr=false

unset OS_TYPE
unset MYFRAME_PARENT_DIR
unset MYFRAME_CURR_DIR
unset MYFRAME_SCRIPT_PATH
