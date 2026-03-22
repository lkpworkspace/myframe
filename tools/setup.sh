#!/usr/bin/env sh
readonly MYFRAME_SCRIPT_PATH="${BASH_SOURCE[0]:-${(%):-%x}}"
readonly MYFRAME_CURR_DIR="$(cd "$(dirname "$MYFRAME_SCRIPT_PATH")" && pwd)"
readonly MYFRAME_PARENT_DIR="$(cd "$MYFRAME_CURR_DIR/.." && pwd)"

export LD_LIBRARY_PATH=${MYFRAME_PARENT_DIR}/lib:${LD_LIBRARY_PATH}
export PYTHONPATH=${PYTHONPATH}:${MYFRAME_PARENT_DIR}/lib/python
