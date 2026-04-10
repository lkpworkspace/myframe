#!/usr/bin/python3
# -*- coding: UTF-8 -*-

import os
import sys
import platform

def printUsage():
    print('Please run \'source myframe_setup.sh\' before run this script!!!')

def checkLibEnv(lib_dir, env_key):
    try:
        ld_library_path = os.environ[env_key]
        paths = ld_library_path.split(':')
        if lib_dir not in paths:
            printUsage()
    except:
        printUsage()

def setup():
    script_dir = os.path.split(os.path.realpath(__file__))[0]
    root_dir = os.path.dirname(script_dir)
    python_lib_dir = os.path.join(root_dir, 'lib', 'python')
    os_name = platform.system()
    if os_name == 'Windows':
        lib_dir = os.path.join(root_dir, 'bin')
        # 添加动态库搜索路径
        os.add_dll_directory(lib_dir)
        # 添加python模块搜索路径
        sys.path.insert(0, python_lib_dir)
    elif os_name == 'Linux':
        # 其他系统主要通过myframe_setup.sh脚本设置环境变量
        # 执行脚本前先执行 source myframe_setup.sh
        lib_dir = os.path.join(root_dir, 'lib')
        checkLibEnv(lib_dir, 'LD_LIBRARY_PATH')
    elif os_name == 'Darwin':
        lib_dir = os.path.join(root_dir, 'lib')
        checkLibEnv(lib_dir, 'DYLD_LIBRARY_PATH')
    else:
        printUsage()

    # print(f'script dir: {script_dir}')
    # print(f'root dir: {root_dir}')
    # print(f'lib dir: {lib_dir}')
    # print(f'python lib dir: {python_lib_dir}')

setup()
